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
    uniform float iTime;
    uniform vec2 iResolution;

    float Scale;

    float map(vec3 p){
        p = mod(p, 2.0) - 1.0;
        p = abs(p) - 1.0;
        if (p.x < p.z) p.xz = p.zx;
        if (p.y < p.z) p.yz = p.zy;
        if (p.x < p.y) p.xy = p.yx;
        float s = 1.0;
        for (int i = 0; i < 10; i++) {
            float r2 = 2.0 / clamp(dot(p, p), 0.1, 1.0);
            p = abs(p) * r2 - vec3(0.6, 0.6, 3.5);
            s *= r2;
        }
        Scale = log2(s);
        return length(p) / s;
    }

    void mainImage(out vec4 fragColor, in vec2 fragCoord) {
        vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
        vec3 p, ro = vec3(0.5 + 0.2 * sin(iTime * 0.03), 0.05 * cos(iTime * 0.03), -0.1 * iTime),
        w = normalize(vec3(0.2, sin(iTime * 0.1), -1)),
        u = normalize(cross(w, vec3(0, 1, 0))),
        rd = mat3(u, cross(u, w), w) * normalize(vec3(uv, 2));
        float h = 0.4, d, i;
        for (i = 1.0; i < 100.0; i++) {
            p = ro + rd * h;
            d = map(p);
            if (d < 0.001 || h > 10.0) break;
            h += d;
        }
        fragColor.xyz = mix(vec3(1), vec3(cos(Scale * 2.5 + p * 1.8) * 0.5 + 0.5), 0.5) * 10.0 / i;
        if (i < 5.0) fragColor.xyz = vec3(0.5, 0.2, 0.1) * (5.0 - i);
        fragColor.a = 1.0;
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

  const config = {
    uniforms: {
      // ...
      iTime: { value: 0.0 },
      iResolution: { value: new THREE.Vector2(400, 600) },
    },
    vertexShader: vertexShader,
    fragmentShader: fragmentShader,
    side: THREE.DoubleSide,
  };

  let shaderMaterial = new THREE.ShaderMaterial(config);
  const planeGeometry = new THREE.PlaneBufferGeometry(50, 50);

  const planeMesh = new THREE.Mesh(planeGeometry, shaderMaterial);
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

    // Update uniforms here if needed
    shaderMaterial.uniforms.iTime.value = performance.now() / 1000;

    renderer.render(scene, camera);
  };

  render();
}
