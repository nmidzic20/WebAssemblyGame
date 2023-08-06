#include <iostream>
#include <cstdio>
#include <cmath>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "context.h"
#include "renderer.h"
#include <emscripten.h> // g++/gcc does not know where this header is, emcc does, so this line should be here only if compiling with emscripten

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>

using namespace std;

int load_image(string image_path, context *ctx) {
    SDL_Texture* texture = IMG_LoadTexture(ctx->renderer, image_path.c_str());
    if (!texture) {
        throw runtime_error("Failed to load image: " + image_path + "\nError: " + SDL_GetError());
    }

    ctx->images.push_back(texture);
    return ctx->images.size() - 1;
}

void update_collidables(context *ctx) {

    for (auto &collidable : ctx->collidables) {
        collidable.x -= ctx->scroll_speed;
        if (collidable.x + collidable.width < 0.0f) {
            // If the collidable has scrolled completely out of view, reset its position on the right side with new random value
            collidable.x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
            collidable.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));

            collidable.collided = false;
        }
    }
}

void update_background_offset(context *ctx) {
    ctx->background_offset += ctx->scroll_speed;
    if (ctx->background_offset >= ctx->WINDOW_WIDTH) {//ctx->background_image_width - ctx->WINDOW_WIDTH) {
        ctx->background_offset = 0.0f;
    }
}

bool check_collision(const SDL_Rect& rect1, const SDL_Rect& rect2) {
    return (
        rect1.x < rect2.x + rect2.w &&
        rect1.x + rect1.w > rect2.x &&
        rect1.y < rect2.y + rect2.h &&
        rect1.y + rect1.h > rect2.y
    );
}

void init_collidables(context *ctx) {
    for (int i = 0; i < ctx->NUMBER_COLLIDABLES; i++) {
        context::Collidable coin;
        coin.x = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
        coin.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
        coin.enemy = false;
        ctx->collidables.push_back(coin);

        context::Collidable enemy;
        enemy.x = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
        enemy.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
        enemy.enemy = true;
        ctx->collidables.push_back(enemy);
    }
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void handleInput(int key, int state, context* ctx) {
        switch (key) {
            case 'M':
            case 'm':
                //if (state == 1)
                    ctx->is_yellow = !ctx->is_yellow;
                break;
            case 38:
            case 'W':
            case 'w':
                //if (state == 1)
                    ctx->cube_position.y = max(ctx->cube_position.y - 10, ctx->cube_size / 2);
                break;
            case 40:
            case 'S':
            case 's':
                //if (state == 1)
                    ctx->cube_position.y = min(ctx->cube_position.y + 10, ctx->WINDOW_HEIGHT - ctx->cube_size / 2);
                break;
            case 37:
            case 'A':
            case 'a':
                //if (state == 1)
                    ctx->cube_position.x = max(ctx->cube_position.x - 10, ctx->cube_size / 2);
                break;
            case 39:
            case 'D':
            case 'd':
                //if (state == 1)
                    ctx->cube_position.x = min(ctx->cube_position.x + 10, ctx->WINDOW_WIDTH - ctx->cube_size / 2);
                break;
        }
    }
    
}

void handle_collisions(context *ctx) {
    SDL_Rect cubeRect = {static_cast<int>(ctx->cube_position.x - ctx->cube_size / 2), static_cast<int>(ctx->cube_position.y - ctx->cube_size / 2), static_cast<int>(ctx->cube_size), static_cast<int>(ctx->cube_size)};

    for (auto &collidable : ctx->collidables) {
        // Only check for collisions if the collidable hasn't collided before
        if (!collidable.collided) {
            SDL_Rect collidableRect = {static_cast<int>(collidable.x), static_cast<int>(collidable.y - collidable.height), static_cast<int>(collidable.width), static_cast<int>(collidable.height)};
            
            if (check_collision(cubeRect, collidableRect)) {
                // Logic for both coins and enemies when collision with cube happens
                if (collidable.enemy) {
                    ctx->prevCollision = true;
                    if (--ctx->lives <= 0) {
                        ctx->scroll_speed = 0.0f;
                    }
                } else {
                    ctx->score++;
                    collidable.x = ctx->WINDOW_WIDTH + static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_WIDTH));
                    collidable.y = static_cast<float>(rand() % static_cast<int>(ctx->WINDOW_HEIGHT));
                }

                // Reset the collided flag for this collidable to block further detections
                collidable.collided = true;
            } else {
                // Cube is not colliding with this collidable
                // This is to re-enable collision with the same collidable once the cube has already collided with it, and that collidable is still rendered on-screen
                ctx->prevCollision = false;
            }
        }
    }

    // Reset the collided flag for the collidable if the cube is not colliding with it anymore
    if (!ctx->prevCollision) {
        for (auto &collidable : ctx->collidables) {
            SDL_Rect collidableRect = {static_cast<int>(collidable.x), static_cast<int>(collidable.y - collidable.height), static_cast<int>(collidable.width), static_cast<int>(collidable.height)};
            if (!check_collision(cubeRect, collidableRect)) {
                collidable.collided = false;
            }
        }
    }
}

void render_frame(context *ctx) {
    Renderer::draw_background(ctx);

    if (ctx->lives > 0) {
        Renderer::draw_cube(ctx);
    }

    Renderer::draw_collidables(ctx);
    Renderer::draw_text(ctx);
}

void main_loop(void *arg) {
    context *ctx = static_cast<context*>(arg);

    //SDL_GL_MakeCurrent(ctx->window, ctx->opengl_context);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cout << "OpenGL error: " << error << endl;
    }

    SDL_GL_SwapWindow(ctx->window);

    /*render_frame(ctx);
    handle_collisions(ctx);

    SDL_RenderPresent(ctx->renderer);

    update_background_offset(ctx);
    update_collidables(ctx);*/

}

GLuint createShader(const char* source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for shader compilation errors (optional but helpful for debugging)
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        cerr << "Error: Shader compilation failed\n" << infoLog << endl;
    }

    return shader;
}

GLuint createProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    //apply shader!!
    glUseProgram(program);

    // Check for program linking errors (optional but helpful for debugging)
    /*GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cerr << "Error: Program linking failed\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);*/

    return program;
}

void init_geometry(GLuint *shaderProgram)
{
    // Create vertex buffer object and copy vertex data into it
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLfloat vertices[] = 
    {
        0.0f, 50.0f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Specify the layout of the shader vertex data (positions only, 3 floats)
    GLint posAttrib = glGetAttribLocation(*shaderProgram, "position");
    if (posAttrib == -1) {
        cout << "Error: 'position' attribute not found in the vertex shader." << endl;
        return;
    }
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        cout << "OpenGL error during vertex attribute setup: " << error << endl;
    }
}


int main(int argc, char *argv[]) {
    context ctx;

    SDL_Init(SDL_INIT_EVERYTHING);

    ctx.window = SDL_CreateWindow("Endless Runner", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    ctx.renderer = SDL_CreateRenderer(ctx.window, -1, SDL_WINDOW_OPENGL); // SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // Create OpenGLES 2 context on SDL window
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    ctx.opengl_context = SDL_GL_CreateContext(ctx.window);
    //glViewport(0, 0, ctx.WINDOW_WIDTH, ctx.WINDOW_HEIGHT);

    // Shader sources
    const GLchar* vertexSource =
        "attribute vec4 position;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vec4(position.xyz, 1.0);  \n"
        "}                            \n";
    const GLchar* fragmentSource =
        "precision mediump float;\n"
        "void main()                                  \n"
        "{                                            \n"
        "  gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );\n"
        "}                                            \n";


    GLuint vertexShader = createShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader(fragmentSource, GL_FRAGMENT_SHADER);
    ctx.program = createProgram(vertexShader, fragmentShader);
    init_geometry(&ctx.program);

    try {
        ctx.background_image = load_image("./assets/image.png", &ctx);
    }
    catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }
    SDL_QueryTexture(ctx.images[ctx.background_image], nullptr, nullptr, &ctx.background_image_width, &ctx.background_image_height);

    try {
        TTF_Init();
        ctx.font = TTF_OpenFont("./assets/Ranchers-Regular.ttf", ctx.font_size);
    } catch (const exception &e) {
        cerr << "Error: " << e.what() << endl;
    }

    init_collidables(&ctx);

    bool running = true;
    SDL_Event event;

    EM_ASM(
        Module.handleEvent = function(event) {
            if (event.type === "keydown") {
                Module.ccall('handleInput', 'void', ['int', 'int', 'number'], [event.keyCode, 1, $0]);
            }
        };

        document.addEventListener('keydown', Module.handleEvent);
    , &ctx);

    emscripten_set_main_loop_arg(main_loop, &ctx, -1, 1);

    SDL_DestroyRenderer(ctx.renderer);
    SDL_DestroyWindow(ctx.window);
    SDL_Quit();

    return 0;
}
