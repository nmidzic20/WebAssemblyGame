import * as THREE from "https://cdnjs.cloudflare.com/ajax/libs/three.js/r127/three.module.js";

window.addEventListener("load", () => {
  if (avatarContainer) {
    setupShapes(avatarContainer);
  }
});

function setupShapes(container) {
  const scene = new THREE.Scene();
  scene.background = new THREE.Color("#0d0c18");

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

  const camera = new THREE.PerspectiveCamera();
  camera.position.set(10, 10, 60);
  camera.lookAt(0, 0, 0);

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
        console.log("Cube was clicked");
      } else if (intersects[0].object === cone) {
        console.log("Cone was clicked");
      }
    }
  };

  renderer.domElement.addEventListener("click", onMouseClick);
  renderer.domElement.addEventListener("mousemove", onMouseMove);
  renderer.domElement.addEventListener("mouseout", onMouseOut);

  const render = () => {
    requestAnimationFrame(render);

    cube.rotation.y += 0.01;
    cone.rotation.x -= 0.01;
    cone.rotation.y -= 0.01;

    renderer.render(scene, camera);
  };

  render();
}
