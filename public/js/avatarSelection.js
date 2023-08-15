import * as THREE from "https://cdnjs.cloudflare.com/ajax/libs/three.js/r127/three.module.js";

let selectedAvatar = "";
let avatarShape;
let canvasWidth = 600;
let canvasHeight = 600;

fetch("./shaders/vertexShader.glsl")
  .then((response) => response.text())
  .then((vertexShaderCode) => {
    fetch("./shaders/fragmentShader.glsl")
      .then((response) => response.text())
      .then((fragmentShaderCode) => {
        setupShapes(avatarContainer, vertexShaderCode, fragmentShaderCode);
      });
  })
  .catch((error) => console.error("Error loading shader files:", error));

function setupShapes(container, vertexShader, fragmentShader) {
  const scene = new THREE.Scene();
  const camera = new THREE.PerspectiveCamera();
  camera.position.set(0, 0, 60);
  camera.lookAt(0, 0, 0);

  const config = {
    uniforms: {
      iTime: { value: 0.0 },
      iResolution: { value: new THREE.Vector2(canvasWidth, canvasHeight) },
    },
    vertexShader: vertexShader,
    fragmentShader: fragmentShader,
    side: THREE.DoubleSide,
  };

  let shaderMaterial = new THREE.ShaderMaterial(config);
  const planeGeometry = new THREE.PlaneBufferGeometry(80, 80);

  const planeMesh = new THREE.Mesh(planeGeometry, shaderMaterial);
  planeMesh.position.set(0, 0, -10);
  scene.add(planeMesh);

  const geometryCube = new THREE.BoxGeometry(10, 10, 10);
  geometryCube.center();
  const materialCube = new THREE.MeshNormalMaterial({
    side: THREE.DoubleSide,
  });
  const cube = new THREE.Mesh(geometryCube, materialCube);
  scene.add(cube);

  const geometryCone = new THREE.ConeGeometry(5, 20, 32);
  const materialCone = new THREE.MeshNormalMaterial({
    side: THREE.DoubleSide,
  });
  const cone = new THREE.Mesh(geometryCone, materialCone);
  cone.position.x = 15;
  scene.add(cone);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  container.appendChild(renderer.domElement);
  renderer.setSize(canvasWidth, canvasHeight);
  renderer.setPixelRatio(window.devicePixelRatio);

  const raycaster = new THREE.Raycaster();
  const mouse = new THREE.Vector2();

  const originalMaterial = cube.material;

  const onMouseMove = (event) => {
    const rect = renderer.domElement.getBoundingClientRect();
    mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    raycaster.setFromCamera(mouse, camera);

    const intersects = raycaster.intersectObjects([cube, cone]);

    if (intersects.length > 0) {
      const hoveredObject = intersects[0].object;
      hoveredObject.material = new THREE.MeshBasicMaterial({ color: 0xff0000 });
    } else {
      cube.material = originalMaterial;
      cone.material = originalMaterial;
    }
  };

  const onMouseOut = () => {
    cube.material = originalMaterial;
    cone.material = originalMaterial;
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

    // Update neccesary shader uniforms
    shaderMaterial.uniforms.iTime.value = performance.now() / 1000;

    renderer.render(scene, camera);
  };

  render();
}
