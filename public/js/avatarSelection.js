import * as THREE from "three";
import { OBJLoader } from "three/addons/loaders/OBJLoader.js";

let canvasWidth = 600;
let canvasHeight = 600;

let bullets = [];
let hightlightMaterial = new THREE.MeshBasicMaterial({
  color: new THREE.Color(0x00aaff),
  transparent: true,
  opacity: 0.5,
});
let models = [];
let materials = [];
let ship1, ship2, ship3;
let selectedAvatar = "";
let avatarShape;
let avatarPosition = new THREE.Vector3(0, -10, 0);

let scene;
let camera;
let renderer;
let raycaster;
let mouse;

let shaderMaterial;

Promise.all([
  loadShader("./shaders/vertexShader.glsl"),
  loadShader("./shaders/fragmentShader.glsl"),
])
  .then(([vertexShaderCode, fragmentShaderCode]) => {
    renderScene(avatarContainer, vertexShaderCode, fragmentShaderCode);
  })
  .catch((error) => console.error("Error loading shader files:", error));

async function loadShader(shaderPath) {
  return fetch(shaderPath).then((response) => response.text());
}

function renderScene(container, vertexShader, fragmentShader) {
  initScene();
  renderShader(vertexShader, fragmentShader);
  loadModels();

  renderer = new THREE.WebGLRenderer({ antialias: true });
  container.appendChild(renderer.domElement);
  renderer.setSize(canvasWidth, canvasHeight);
  renderer.setPixelRatio(window.devicePixelRatio);

  raycaster = new THREE.Raycaster();
  mouse = new THREE.Vector2();

  setEventListeners();

  render();
}

function initScene() {
  scene = new THREE.Scene();
  camera = new THREE.PerspectiveCamera();
  camera.position.set(0, 0, 60);
  camera.lookAt(0, 0, 0);

  // Add ambient light to the scene - otherwise objects from .obj file appear as black
  const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
  scene.add(ambientLight);

  // Add directional light to the scene - otherwise objects from .obj file appear as black
  const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
  directionalLight.position.set(1, 1, 1).normalize();
  scene.add(directionalLight);
}

function renderShader(vertexShader, fragmentShader) {
  const config = {
    uniforms: {
      iTime: { value: 0.0 },
      iResolution: { value: new THREE.Vector2(canvasWidth, canvasHeight) },
    },
    vertexShader: vertexShader,
    fragmentShader: fragmentShader,
    side: THREE.DoubleSide,
  };

  shaderMaterial = new THREE.ShaderMaterial(config);
  const planeGeometry = new THREE.PlaneGeometry(80, 80);

  // Background plane to display shader
  const planeMesh = new THREE.Mesh(planeGeometry, shaderMaterial);
  planeMesh.position.set(0, 0, -10);
  scene.add(planeMesh);
}

function loadModels() {
  const loader = new OBJLoader();
  loader.load(
    "../assets/models/ship/prometheus.obj",
    (object) => createShipModels(object),
    (xhr) => {
      //console.log((xhr.loaded / xhr.total) * 100 + "% loaded");
    },
    (error) => {
      console.error("An error occurred", error);
    }
  );
}

function setEventListeners() {
  renderer.domElement.addEventListener("click", onMouseClick);
  renderer.domElement.addEventListener("mousemove", onMouseMove);
  renderer.domElement.addEventListener("mouseout", onMouseOut);

  document.addEventListener("keydown", (event) => handleKeydown(event));
}

function render() {
  requestAnimationFrame(render);

  if (playing === true) {
    if (models.length == 3) {
      if (ship1.position.x != -20) {
        setModelsSelection();
      }
      setAvatarModel();
    }
  } else {
    if (models.length == 3) {
      if (ship1.position.x != 0) {
        if (avatarShape) scene.remove(avatarShape);

        resetModelsPosition();
        resetModelsRotation();
        setModelsScale({ x: 4, y: 4, z: 4 });
      }

      models.forEach((model) => (model.rotation.y += 0.01));
    }
  }

  // Update neccesary shader uniforms (time)
  shaderMaterial.uniforms.iTime.value = performance.now() / 1000;

  for (const bullet of bullets) {
    bullet.position.z -= 4;
    bullet.position.y += 0.3;
    if (bullet.position.z < -100) {
      // Remove the bullet if it's far enough from the avatar
      scene.remove(bullet);
      bullets.splice(bullets.indexOf(bullet), 1);
    }
  }

  renderer.render(scene, camera);
}

function setModelsSelection() {
  let startX = -20;
  let modelData = models.map((model, index) => {
    return {
      model: model,
      positionX: startX + index * 10,
      modelName: "ship" + (index + 1),
    };
  });

  modelData.forEach((data) => {
    const { model, positionX, modelName } = data;

    model.rotation.set(0, 0, 0);
    model.position.set(positionX, 22, 0);
    model.scale.set(1.5, 1.5, 1.5);
    model.getObjectByName("Plane.100_Plane.102").userData.modelName = modelName;
  });
}

function setAvatarModel() {
  const lastSelectedShape =
    selectedAvatar === "ship1"
      ? ship1
      : selectedAvatar === "ship2"
      ? ship2
      : ship3;

  if (avatarShape) scene.remove(avatarShape);
  avatarShape = lastSelectedShape.clone();

  avatarShape.scale.set(4, 4, 4);
  avatarShape.position.set(0, 0, 0);
  avatarShape.rotation.y += 0.01;

  scene.add(avatarShape);
  avatarShape.position.add(avatarPosition);
}

function createBullet() {
  const bulletGeometry = new THREE.SphereGeometry(1.5, 8, 8);
  const bulletMaterial = new THREE.MeshBasicMaterial({
    color: 0xffa500,
    depthTest: false,
    depthWrite: false,
  });
  const bulletMesh = new THREE.Mesh(bulletGeometry, bulletMaterial);

  bulletMesh.position.set(
    avatarPosition.x,
    avatarPosition.y + 4,
    avatarPosition.z - 20
  );
  scene.add(bulletMesh);
  bullets.push(bulletMesh);
}

function resetModelsPosition() {
  ship1.position.set(0, -15, 0);
  ship2.position.set(0, 15, 0);
  ship3.position.set(0, 0, 0);
}

function resetModelsRotation() {
  ship1.rotation.set(0, Math.PI / 2, 0);
  ship2.rotation.set(0, Math.PI / 2, 0);
  ship3.rotation.set(0, Math.PI / 2, 0);
}

function setModelsScale(scale) {
  ship1.scale.set(scale.x, scale.y, scale.z);
  ship2.scale.set(scale.x, scale.y, scale.z);
  ship3.scale.set(scale.x, scale.y, scale.z);
}

function createShipModels(object) {
  const materialShip1 = new THREE.MeshToonMaterial({
    color: new THREE.Color(0xffff00),
  });
  const materialShip2 = new THREE.MeshPhongMaterial({
    color: 0x999999,
    specular: 0xffffff,
    shininess: 100,
  });
  const materialShip3 = new THREE.MeshNormalMaterial({
    side: THREE.DoubleSide,
  });
  materials.push(materialShip1);
  materials.push(materialShip2);
  materials.push(materialShip3);

  ship1 = object.clone();
  ship2 = object.clone();
  ship3 = object.clone();

  models.push(ship1);
  models.push(ship2);
  models.push(ship3);

  models.forEach((model, index) => {
    model.name = "ship" + (index + 1);
    applyMaterialToModel(model, materials[index], index);
  });

  resetModelsPosition();
  resetModelsRotation();
  setModelsScale({ x: 4, y: 4, z: 4 });

  models.forEach((model) => scene.add(model));
}

function applyMaterialToModel(model, material, index) {
  model.traverse((child) => {
    if (child instanceof THREE.Mesh) {
      child.material = material;
      if (!child.userData.modelName)
        child.userData.modelName = "ship" + (index + 1);
    }
  });
}

function onMouseMove(event) {
  const rect = renderer.domElement.getBoundingClientRect();
  mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
  mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

  raycaster.setFromCamera(mouse, camera);

  const intersects = raycaster.intersectObjects(models);

  if (intersects.length > 0) {
    const hoveredObject = intersects[0].object;
    const modelName = hoveredObject.userData.modelName;

    let model = models.find((model) => model.name === modelName);

    if (model) {
      applyMaterialToModel(model, hightlightMaterial, null);
    }
  } else {
    if (models.length == 3) {
      models.forEach((model, index) => {
        applyMaterialToModel(model, materials[index], index);
      });
    }
  }
}

function onMouseOut() {
  if (models.length == 3) {
    models.forEach((model, index) => {
      applyMaterialToModel(model, materials[index], index);
    });
  }
}

function onMouseClick(event) {
  const rect = renderer.domElement.getBoundingClientRect();
  mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
  mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

  raycaster.setFromCamera(mouse, camera);

  const intersects = raycaster.intersectObjects(models);

  if (intersects.length > 0) {
    const clickedObject = intersects[0].object;

    const modelName = clickedObject.userData.modelName;

    changeAvatar(modelName);
  }
}

function changeAvatar(modelName) {
  selectedAvatar = modelName;
  let modelIndex = models.indexOf(
    models.find((model) => model.name === modelName)
  );
  Module.ccall(
    "set_avatar",
    "void",
    ["number", "number"],
    [Module.context, modelIndex]
  );
}

function handleKeydown(event) {
  switch (event.key) {
    case " ":
      createBullet();
      break;
    case "ArrowUp":
    case "w":
    case "W":
      avatarPosition.y += 1;
      break;
    case "ArrowDown":
    case "s":
    case "S":
      avatarPosition.y -= 1;
      break;
    case "ArrowLeft":
    case "a":
    case "A":
      avatarPosition.x -= 1;
      break;
    case "ArrowRight":
    case "d":
    case "D":
      avatarPosition.x += 1;
      break;
    default:
      break;
  }
}
