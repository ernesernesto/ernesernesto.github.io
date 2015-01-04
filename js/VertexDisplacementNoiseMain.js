var renderer, scene, camera, material, start = Date.now();

var container = document.getElementById("container");
scene = new THREE.Scene();
camera = new THREE.PerspectiveCamera(30, window.innerWidth / window.innerHeight, 1, 10000);
camera.position.z = 100;
camera.target = new THREE.Vector3(0, 0, 0);
scene.add(camera);

material = new THREE.ShaderMaterial(
{ 
    uniforms: {
        wave:{
            type : "t",
            value : THREE.ImageUtils.loadTexture("assets/OceanSurface.jpg")
        },
        time: {
            type : "f",
            value : 0.0
        }
    }, 
    vertexShader : loadShader("VertexDisplacementNoiseVS.glsl"), 
    fragmentShader : loadShader("VertexDisplacementNoiseFS.glsl")
});

var mesh = new THREE.Mesh(new THREE.IcosahedronGeometry(20, 4), material);
scene.add(mesh);

renderer = new THREE.WebGLRenderer();
renderer.setSize(window.innerWidth, window.innerHeight);
container.appendChild(renderer.domElement);
render();

        
function render(){
    material.uniforms['time'].value = 0.00025 * (Date.now() - start);
    renderer.render(scene, camera);
    requestAnimationFrame(render);
}