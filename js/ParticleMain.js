var scene = new THREE.Scene();
scene.fog = new THREE.FogExp2( 0x000000, 0.001 );

var camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000 );
var renderer = new THREE.WebGLRenderer({ clearAlpha : 1 });
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setClearColor(new THREE.Color(0, 1));

document.body.appendChild(renderer.domElement);

var particleCount = 5000;
var particles = new THREE.Geometry();
var material = new THREE.PointCloudMaterial({
	size : 20,
    sizeAttenuation: false, 
    map : THREE.ImageUtils.loadTexture("assets/particle.png"),
    transparent: true 
    });

for(var p = 0; p < particleCount; p++){
	var px = Math.random() * 500 - 250;
    var py = Math.random() * 500 - 250;
    var pz = Math.random() * 500 - 250;
    var particle = new THREE.Vector3(px, py, pz);
    particle.velocity = new THREE.Vector3(0, -Math.random(), 0);

    particles.vertices.push(particle);
}

var particleSystem = new THREE.PointCloud(particles, material);
particleSystem.sortParticle = true;
scene.add(particleSystem);

camera.position.z = 5;

render();

function render(){
	particleSystem.rotation.y += 0.01;

	var count = particleCount;
    while(count--){
    	var particle = particles.vertices[count];
        if(particle.y < -200){
        	particle.y = 200;
            particle.velocity.y = 0;
        }

        particle.velocity.y -= Math.random() * 0.1;
        particle.add(particle.velocity);
    }

    particleSystem.geometry.verticesNeedUpdate = true;

    renderer.render(scene, camera);
    requestAnimationFrame(render);
}

