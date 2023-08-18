import * as THREE from "three";
import { OBJLoader } from "three/addons/loaders/OBJLoader.js";

let selectedAvatar = "";
let avatarShape;
let canvasWidth = 600;
let canvasHeight = 600;
let bullets = [];
const highlightColor = new THREE.Color(0x00aaff);

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
  const planeGeometry = new THREE.PlaneGeometry(80, 80);

  // Background plane to display shader
  const planeMesh = new THREE.Mesh(planeGeometry, shaderMaterial);
  planeMesh.position.set(0, 0, -10);
  //scene.add(planeMesh);

  // Add ambient light to the scene - otherwise objects from .obj file appear as black
  const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
  scene.add(ambientLight);

  // Add directional light to the scene - otherwise objects from .obj file appear as black
  const directionalLight = new THREE.DirectionalLight(0xffffff, 1);
  directionalLight.position.set(1, 1, 1).normalize();
  scene.add(directionalLight);

  let ship1, ship2, ship3;

  const loader = new OBJLoader();
  loader.load(
    "../assets/models/ship/prometheus.obj",
    (object) => {
      const materialShip1 = new THREE.MeshToonMaterial({
        color: new THREE.Color(0xffff00), //(0xd2aa6d),
      });
      const materialShip2 = new THREE.MeshPhongMaterial({
        color: 0x999999,
        specular: 0xffffff,
        shininess: 100,
      });
      const materialShip3 = new THREE.MeshNormalMaterial({
        side: THREE.DoubleSide,
      });

      ship1 = object.clone();
      ship2 = object.clone();
      ship3 = object.clone();

      ship1.traverse((child) => {
        if (child instanceof THREE.Mesh) {
          child.material = materialShip1;
          child.userData.shipName = "ship1";
        }
      });
      ship2.traverse((child) => {
        if (child instanceof THREE.Mesh) {
          child.material = materialShip2;
          child.userData.shipName = "ship2";
        }
      });
      ship3.traverse((child) => {
        if (child instanceof THREE.Mesh) {
          child.material = materialShip3;
          child.userData.shipName = "ship3";
        }
      });

      originalMaterialShip1 = materialShip1;
      originalMaterialShip2 = materialShip2;
      originalMaterialShip3 = materialShip3;

      ship1.position.set(0, -15, 0);
      ship1.rotation.set(0, Math.PI / 2, 0);
      ship1.scale.set(4, 4, 4);

      ship2.position.set(0, 15, 0);
      ship2.rotation.set(0, Math.PI / 2, 0);
      ship2.scale.set(4, 4, 4);

      ship3.position.set(0, 0, 0);
      ship3.rotation.set(0, Math.PI / 2, 0);
      ship3.scale.set(4, 4, 4);

      scene.add(ship1);
      scene.add(ship2);
      scene.add(ship3);
    },
    (xhr) => {
      //console.log((xhr.loaded / xhr.total) * 100 + "% loaded");
    },
    (error) => {
      console.error("An error occurred", error);
    }
  );

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  container.appendChild(renderer.domElement);
  renderer.setSize(canvasWidth, canvasHeight);
  renderer.setPixelRatio(window.devicePixelRatio);

  const raycaster = new THREE.Raycaster();
  const mouse = new THREE.Vector2();

  let originalMaterialShip1, originalMaterialShip2, originalMaterialShip3;

  const onMouseMove = (event) => {
    const rect = renderer.domElement.getBoundingClientRect();
    mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    raycaster.setFromCamera(mouse, camera);

    const intersects = raycaster.intersectObjects([ship1, ship2, ship3]);

    if (intersects.length > 0) {
      const hoveredObject = intersects[0].object;
      const shipName = hoveredObject.userData.shipName;

      if (shipName === "ship1") {
        ship1.traverse((child) => {
          if (child instanceof THREE.Mesh) {
            child.material = new THREE.MeshBasicMaterial({
              color: highlightColor,
              transparent: true,
              opacity: 0.5,
            });
            child.userData.shipName = "ship1";
          }
        });
      } else if (shipName === "ship2") {
        ship2.traverse((child) => {
          if (child instanceof THREE.Mesh) {
            child.material = new THREE.MeshBasicMaterial({
              color: highlightColor,
              transparent: true,
              opacity: 0.5,
            });
            child.userData.shipName = "ship2";
          }
        });
      } else if (shipName === "ship3") {
        ship3.traverse((child) => {
          if (child instanceof THREE.Mesh) {
            child.material = new THREE.MeshBasicMaterial({
              color: highlightColor,
              transparent: true,
              opacity: 0.5,
            });
            child.userData.shipName = "ship3";
          }
        });
      }
    } else {
      if (ship1 && ship2 && ship3) {
        ship1.traverse((child) => {
          if (child instanceof THREE.Mesh) {
            child.material = originalMaterialShip1;
          }
        });
        ship2.traverse((child) => {
          if (child instanceof THREE.Mesh) {
            child.material = originalMaterialShip2;
          }
        });
        ship3.traverse((child) => {
          if (child instanceof THREE.Mesh) {
            child.material = originalMaterialShip3;
          }
        });
      }
    }
  };

  const onMouseOut = () => {
    if (ship1 && ship2 && ship3) {
      ship1.traverse((child) => {
        if (child instanceof THREE.Mesh) {
          child.material = originalMaterialShip1;
        }
      });
      ship2.traverse((child) => {
        if (child instanceof THREE.Mesh) {
          child.material = originalMaterialShip2;
        }
      });
      ship3.traverse((child) => {
        if (child instanceof THREE.Mesh) {
          child.material = originalMaterialShip3;
        }
      });
    }
  };

  const onMouseClick = (event) => {
    const rect = renderer.domElement.getBoundingClientRect();
    mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    raycaster.setFromCamera(mouse, camera);

    const intersects = raycaster.intersectObjects([ship1, ship2, ship3]);

    if (intersects.length > 0) {
      const clickedObject = intersects[0].object;
      console.log("Clicked");
      console.log(clickedObject);
      const shipName = clickedObject.userData.shipName;

      if (shipName === "ship1") {
        console.log("clicked ship1");

        selectedAvatar = "ship1";
        Module.ccall(
          "set_avatar",
          "void",
          ["number", "number"],
          [Module.context, 0]
        );
      } else if (shipName === "ship2") {
        console.log("clicked ship2");

        selectedAvatar = "ship2";
        Module.ccall(
          "set_avatar",
          "void",
          ["number", "number"],
          [Module.context, 1]
        );
      } else if (shipName === "ship3") {
        console.log("clicked ship3");

        selectedAvatar = "ship3";
        Module.ccall(
          "set_avatar",
          "void",
          ["number", "number"],
          [Module.context, 2]
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
      case " ":
        const bulletGeometry = new THREE.SphereGeometry(1, 8, 8);
        const bulletMaterial = new THREE.MeshBasicMaterial({ color: 0xffa500 });
        const bulletMesh = new THREE.Mesh(bulletGeometry, bulletMaterial);

        bulletMesh.position.set(
          avatarPosition.x,
          avatarPosition.y,
          avatarPosition.z
        );

        scene.add(bulletMesh);
        bullets.push(bulletMesh);
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
  });

  const render = () => {
    requestAnimationFrame(render);

    for (const bullet of bullets) {
      bullet.position.z += 0.5;
      bullet.position.y -= 0.2;
      if (bullet.position.z < -100) {
        // Remove the bullet if it's far enough from the avatar
        scene.remove(bullet);
        bullets.splice(bullets.indexOf(bullet), 1);
      }
    }

    if (playing === true) {
      if (ship1 && ship2 && ship3) {
        if (ship1.position.x != -20) {
          ship1.rotation.set(0, 0, 0);
          ship1.position.set(-20, 22, 0);
          ship1.scale.set(1.5, 1.5, 1.5);
          ship1.getObjectByName("Plane.100_Plane.102").userData.shipName =
            "ship1";
          console.log("SHIP1");
          console.log(ship1.getObjectByName("Plane.100_Plane.102"));

          ship2.rotation.set(0, 0, 0);
          ship2.position.set(-10, 22, 0);
          ship2.scale.set(1.5, 1.5, 1.5);
          ship2.getObjectByName("Plane.100_Plane.102").userData.shipName =
            "ship2";
          console.log("SHIP2");
          console.log(ship2.getObjectByName("Plane.100_Plane.102"));

          ship3.rotation.set(0, 0, 0);
          ship3.position.set(0, 22, 0);
          ship3.scale.set(1.5, 1.5, 1.5);
          ship3.getObjectByName("Plane.100_Plane.102").userData.shipName =
            "ship3";
          console.log("SHIP3");
          console.log(ship3.getObjectByName("Plane.100_Plane.102"));
        }

        const lastSelectedShape =
          selectedAvatar === "ship1"
            ? ship1
            : selectedAvatar === "ship2"
            ? ship2
            : ship3;

        if (avatarShape != undefined) scene.remove(avatarShape);
        avatarShape = lastSelectedShape.clone();

        avatarShape.scale.set(4, 4, 4);
        avatarShape.position.set(0, 0, 0);
        avatarShape.rotation.y += 0.01;

        scene.add(avatarShape);

        avatarShape.position.add(avatarPosition);
      }
    } else {
      if (ship1 && ship2 && ship3) {
        ship1.rotation.y += 0.01;
        ship2.rotation.y += 0.01;
        ship3.rotation.y += 0.01;
      }
    }

    // Update neccesary shader uniforms (time)
    shaderMaterial.uniforms.iTime.value = performance.now() / 1000;

    renderer.render(scene, camera);
  };

  render();
}
