import * as THREE from "https://cdnjs.cloudflare.com/ajax/libs/three.js/r127/three.module.js";

let selectedAvatar = "";
let avatarShape;
const vertexShader = `
varying vec2 vUv;

void main() {
    vUv = uv;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}

`;

const fragmentShader = `
varying vec2 vUv;

const int MAX_MARCHING_STEPS = 10000;
const float MIN_DIST = 0.0;
const float MAX_DIST = 1000000.0;
const float EPSILON = 0.1;
const float size = 1000.0;
const float scale = EPSILON * 10.0;

vec3 surface_color(vec3 p)
{
    p /= scale * 10.0;
    float color1 = length(sin(p / 100.0)) / 2.0;
    return vec3(color1, color1, 0.0);
}


float planet_surface(vec3 p){
  vec3 p1 = p/size;
  p = sin(vec3(sin(p1.x)/p1.x,cos(p1.y)-p1.y,sin(p1.z)+p1.z))*size;
  return length(p) - size;
}

float sceneSDF(vec3 p) {
  p /= scale;
  float result = planet_surface(p);
  float i = 1.0;
  for(int k = 0; k < 3; k++){
    result = max(result, planet_surface(p*i)/(i));
      i *= 10.0;
  }
  //float result = sceneSDF1(p/1000.0+sceneSDF1(p/1000.0));
  return result*scale;
}

float shortestDistanceToSurface(vec3 eye, vec3 marchingDirection, float start, float end) {
  float depth = start;
  float eps = EPSILON;
  for (int i = 0; i < MAX_MARCHING_STEPS; i++) {
      float dist = sceneSDF(eye + depth * marchingDirection);
      if (dist < eps) {
    return depth;
      }
      depth += dist;
      eps *= 1.01;
      if (depth >= end) {
          return end;
      }
  }
  return end;
}
          
vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
  vec2 xy = fragCoord - size / 2.0;
  float z = size.y / tan(radians(fieldOfView) / 2.0);
  return normalize(vec3(xy, -z));
}

vec3 estimateNormal(vec3 p) {
  return normalize(vec3(
      sceneSDF(vec3(p.x + EPSILON, p.y, p.z)) - sceneSDF(vec3(p.x - EPSILON, p.y, p.z)),
      sceneSDF(vec3(p.x, p.y + EPSILON, p.z)) - sceneSDF(vec3(p.x, p.y - EPSILON, p.z)),
      sceneSDF(vec3(p.x, p.y, p.z  + EPSILON)) - sceneSDF(vec3(p.x, p.y, p.z - EPSILON))
  ));
}

vec3 phongContribForLight(vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye,
                        vec3 lightPos, vec3 lightIntensity) {
  lightPos = eye;
  vec3 N = estimateNormal(p);
  vec3 L = normalize(lightPos - p);
  vec3 V = normalize(eye - p);
  vec3 R = normalize(reflect(-L, N));
  
  float dotLN = dot(L, N);
  float dotRV = dot(R, V);
  
  if (dotLN < 0.0) {
      // Light not visible from this point on the surface
      return vec3(0.0, 0.0, 0.0);
  } 
  
  if (dotRV < 0.0) {
      // Light reflection in opposite direction as viewer, apply only diffuse
      // component
      return lightIntensity * (k_d * dotLN);
  }
  return lightIntensity * (k_d * dotLN + k_s * pow(dotRV, alpha));
}

vec3 phongIllumination(vec3 k_a, vec3 k_d, vec3 k_s, float alpha, vec3 p, vec3 eye) {
  const vec3 ambientLight = 0.5 * vec3(1.0, 1.0, 1.0);
  vec3 color = ambientLight * k_a;
  
  vec3 light1Pos = eye;
  vec3 light1Intensity = vec3(0.4, 0.4, 0.4);
  
  color += phongContribForLight(k_d, k_s, alpha, p, eye,
                                light1Pos,
                                light1Intensity);
  
  //vec3 light2Pos = vec3(2.0 * sin(0.37),
  //                      2.0 * cos(0.37),
  //                      2.0);
  //vec3 light2Intensity = vec3(0.4, 0.4, 0.4);
  
  //color += phongContribForLight(k_d, k_s, alpha, p, eye,
  //                              light2Pos,
  //                              light2Intensity);    
  return color;
}

mat3 calculateViewMatrix(vec3 eye, vec3 center, vec3 up) {
  // Based on gluLookAt man page
  vec3 f = normalize(center - eye);
  vec3 s = normalize(cross(f, up));
  vec3 u = cross(s, f);
  return mat3(s, u, -f);
}

uniform vec3 eye;
uniform vec3 lookAt;
uniform float iTime;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
  vec3 viewDir = normalize(lookAt - eye);
    float speed = 10.0;
    vec3 eye = scale * vec3(cos(iTime) / 3.0, sin(iTime) / 1.8, iTime * speed) * size;

    mat3 viewToWorld = calculateViewMatrix(eye, vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));

    vec3 worldDir = viewToWorld * viewDir;

    float dist = shortestDistanceToSurface(eye, worldDir, MIN_DIST, MAX_DIST);

    if (dist > MAX_DIST - EPSILON) {
        // Didn't hit anything
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    vec3 p = eye + dist * worldDir;

    vec3 K_a = surface_color((p));
    vec3 K_d = K_a;
    vec3 K_s = vec3(1.0, 1.0, 1.0);
    float shininess = 10.0;

    vec3 color = phongIllumination(K_a, K_d, K_s, shininess, p, eye);

    fragColor = vec4(color, 1.0);
}

void main() {
    mainImage(gl_FragColor, gl_FragCoord.xy);
}


`;

window.addEventListener("load", () => {
  if (avatarContainer) {
    setupShapes(avatarContainer);
  }
});

function setupShapes(container) {
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera();
  camera.position.set(10, 10, 60);
  camera.lookAt(0, 0, 0);
  //scene.background = new THREE.Color("#0d0c18");

  const showSTConfig = {
    uniforms: {
      eye: { value: camera.position },
      lookAt: { value: new THREE.Vector3() },
      iTime: { value: 0.0 },
    },
    vertexShader: vertexShader,
    fragmentShader: fragmentShader,
    side: THREE.DoubleSide,
  };

  let showSTMaterial = new THREE.ShaderMaterial(showSTConfig);
  /*const boxMesh = new THREE.Mesh(
    new THREE.BoxBufferGeometry(16 * 3, (16 / 4) * 3, (16 / 2) * 3, 4, 4, 4),
    showSTMaterial
  );
  scene.add(boxMesh);*/
  const planeGeometry = new THREE.PlaneBufferGeometry(50, 50); // Set the size as needed

  const planeMesh = new THREE.Mesh(planeGeometry, showSTMaterial);
  scene.add(planeMesh);

  const geometryCube = new THREE.BoxGeometry(10, 10, 10);
  geometryCube.center();
  const materialCube = new THREE.MeshBasicMaterial({ color: 0xffff00 }); //new THREE.MeshNormalMaterial({
  //side: THREE.DoubleSide,
  //});
  const cube = new THREE.Mesh(geometryCube, materialCube);
  scene.add(cube);

  const geometryCone = new THREE.ConeGeometry(5, 20, 32);
  const materialCone = new THREE.MeshBasicMaterial({ color: 0xffff00 }); //new THREE.MeshNormalMaterial({
  //side: THREE.DoubleSide,
  //});
  const cone = new THREE.Mesh(geometryCone, materialCone);
  cone.position.x = 15;
  scene.add(cone);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  container.appendChild(renderer.domElement);
  renderer.setSize(400, 600);
  renderer.setPixelRatio(window.devicePixelRatio);

  const raycaster = new THREE.Raycaster();
  const mouse = new THREE.Vector2();

  const originalCubeColor = cube.material.color.getHex();
  const originalConeColor = cone.material.color.getHex();

  const onMouseMove = (event) => {
    const rect = renderer.domElement.getBoundingClientRect();
    mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    raycaster.setFromCamera(mouse, camera);

    const intersects = raycaster.intersectObjects([cube, cone]);

    if (intersects.length > 0) {
      const hoveredObject = intersects[0].object;

      hoveredObject.material.color.setHex(0xff0000);
    } else {
      cube.material.color.setHex(originalCubeColor);
      cone.material.color.setHex(originalConeColor);
    }
  };

  const onMouseOut = () => {
    cube.material.color.setHex(originalCubeColor);
    cone.material.color.setHex(originalConeColor);
  };

  const onMouseClick = (event) => {
    const rect = renderer.domElement.getBoundingClientRect();
    mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    raycaster.setFromCamera(mouse, camera);

    const intersects = raycaster.intersectObjects([cube, cone]);

    if (intersects.length > 0) {
      if (intersects[0].object === cube) {
        selectedAvatar = "cube";
        Module.ccall(
          "set_avatar",
          "void",
          ["number", "number"],
          [Module.context, 0]
        );
      } else if (intersects[0].object === cone) {
        selectedAvatar = "cone";
        Module.ccall(
          "set_avatar",
          "void",
          ["number", "number"],
          [Module.context, 1]
        );
      }
    }
  };

  renderer.domElement.addEventListener("click", onMouseClick);
  renderer.domElement.addEventListener("mousemove", onMouseMove);
  renderer.domElement.addEventListener("mouseout", onMouseOut);

  let avatarPosition = new THREE.Vector3(0, 0, 0);
  document.addEventListener("keydown", (event) => {
    switch (event.key) {
      case "ArrowUp":
        avatarPosition.y += 1;
        break;
      case "ArrowDown":
        avatarPosition.y -= 1;
        break;
      case "ArrowLeft":
        avatarPosition.x -= 1;
        break;
      case "ArrowRight":
        avatarPosition.x += 1;
        break;
      default:
        break;
    }
  });

  const render = () => {
    requestAnimationFrame(render);

    if (playing === true) {
      cube.rotation.set(0, 0, 0);
      cone.rotation.set(0, 0, 0);

      cube.position.set(-20, 22, 0);
      cube.scale.set(0.5, 0.5, 0.5);

      cone.position.set(-10, 22, 0);
      cone.scale.set(0.5, 0.3, 0.5);

      const lastSelectedShape = selectedAvatar === "cube" ? cube : cone;

      if (avatarShape != undefined) scene.remove(avatarShape);
      avatarShape = lastSelectedShape.clone();
      avatarShape.scale.set(1, 1, 1);
      avatarShape.position.set(0, 0, 0);

      scene.add(avatarShape);

      avatarShape.position.add(avatarPosition);
    } else {
      cube.rotation.y += 0.01;
      cone.rotation.x -= 0.01;
      cone.rotation.y -= 0.01;
    }

    showSTMaterial.uniforms.eye.value.copy(camera.position);
    camera.getWorldDirection(showSTMaterial.uniforms.lookAt.value);
    showSTMaterial.uniforms.iTime.value = performance.now() / 1000;

    renderer.render(scene, camera);
  };

  render();
}
