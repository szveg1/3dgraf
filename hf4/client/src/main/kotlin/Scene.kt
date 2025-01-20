import org.w3c.dom.HTMLCanvasElement
import org.w3c.dom.HTMLDivElement
import vision.gears.webglmath.Mat4
import vision.gears.webglmath.UniformProvider
import vision.gears.webglmath.Vec3
import kotlin.js.Date
import kotlin.math.*
import kotlin.random.Random
import org.khronos.webgl.WebGLRenderingContext as GL

class Scene(
    val gl: WebGL2RenderingContext,
    val overlay: HTMLDivElement,
    val canvas: HTMLCanvasElement
) : UniformProvider("scene") {

    lateinit var defaultFramebuffer: DefaultFramebuffer
    lateinit var framebuffers: Array<Framebuffer>
    var frameCount = 0

    val vsQuad = Shader(gl, GL.VERTEX_SHADER, "quad-vs.glsl")
    val fsBackground = Shader(gl, GL.FRAGMENT_SHADER, "background-fs.glsl")

    val quadGeometry = TexturedQuadGeometry(gl)

    val backgroundProgram = Program(gl, vsQuad, fsBackground)
    val backgroundMaterial = Material(backgroundProgram)
    val skyCubeTexture = TextureCube(
        gl,
        "media/cubemap/px.png", "media/cubemap/nx.png",
        "media/cubemap/py.png", "media/cubemap/ny.png",
        "media/cubemap/pz.png", "media/cubemap/nz.png"
    )

    init {
        backgroundMaterial["envTexture"]?.set(skyCubeTexture)
    }

    val backgroundMesh = Mesh(backgroundMaterial, quadGeometry)

    val jsonLoader = JsonLoader()
    val vsTransform = Shader(gl, GL.VERTEX_SHADER, "transform-vs.glsl")
    val fsPhong = Shader(gl, GL.FRAGMENT_SHADER, "phong-fs.glsl")
    val phongProgram = Program(gl, vsTransform, fsPhong)

    val brickMaterial = Material(phongProgram).apply {
        this["diffuseMap"]?.set(
            Texture2D(gl, "media/brick/Color.jpg")
        )
        this["normalMap"]?.set(
            Texture2D(gl, "media/brick/NormalGL.jpg")
        )
        this["ks"]?.set(Vec3(0.5f, 0.5f, 0.5f))
        this["shininess"]?.set(32f)
    }
    val metalMaterial = Material(phongProgram).apply {
        this["diffuseMap"]?.set(
            Texture2D(gl, "media/metal/Color.jpg")
        )
        this["normalMap"]?.set(
            Texture2D(gl, "media/metal/NormalGL.jpg")
        )
        this["ks"]?.set(Vec3(0.5f, 0.5f, 0.5f))
        this["shininess"]?.set(32f)
    }
    val fsEnvmapped = Shader(gl, GL.FRAGMENT_SHADER, "envmapped-fs.glsl")
    val envmappedProgram = Program(gl, vsTransform, fsEnvmapped)

    val envmappedMaterial = Material(envmappedProgram).apply {
        this["kr"]?.set(Vec3(0.4f, 0.4f, 0.3f))
    }

    val mirror = GameObject(
        Mesh(envmappedMaterial, quadGeometry)
    ).apply {
        scale.set(10f, -10f, 1f)
        pitch = PI.toFloat() / 2f
    }


    val sphereGeometry = jsonLoader.loadGeometries(gl, "media/json/sphere.json")

    fun createSphere(): GameObject {
        return GameObject(
            Mesh(brickMaterial, sphereGeometry[0])
        ).apply {
            position.set(
                Vec3(
                    Random.nextDouble(-10.0, 10.0).toFloat(),
                    Random.nextDouble(2.0, 10.0).toFloat(),
                    Random.nextDouble(-10.0, 10.0).toFloat()
                )
            )
        }

    }

    val lightSphere = GameObject(
        Mesh(metalMaterial, sphereGeometry[0])
    ).apply {
        scale.set(0.1f, 0.1f, 0.1f)
    }


    val gameObjects = ArrayList<GameObject>()

    init {
        gameObjects += mirror
        gameObjects += lightSphere
        for (i in 0 until 10) {
            gameObjects += createSphere()
        }
    }

    val lights = Array<Light>(1) { Light(it) }

    init {
        lights[0].powerDensity.set(20f, 20f, 20f)
    }

    val spotLights = Array<SpotLight>(1) { SpotLight(it) }

    init {
        spotLights[0].powerDensity.set(1f, 1f, 1f)
    }

    val camera = PerspectiveCamera()
    val mirrorCamera = PerspectiveCamera()

    val timeAtFirstFrame = Date().getTime()
    var timeAtLastFrame = timeAtFirstFrame

    init {
        gl.enable(GL.DEPTH_TEST)
    }


    fun resize(gl: WebGL2RenderingContext, canvas: HTMLCanvasElement) {
        gl.viewport(0, 0, canvas.width, canvas.height)
        camera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())
        mirrorCamera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())
        mirrorCamera.pitch = camera.pitch

        defaultFramebuffer = DefaultFramebuffer(canvas.width, canvas.height)
        framebuffers = arrayOf(
            Framebuffer(gl, 1, canvas.width, canvas.height, GL.RGBA32F, GL.RGBA, GL.FLOAT)
        )
        envmappedMaterial["environment"]?.set(framebuffers[0].targets[0])
    }

    @Suppress("UNUSED_PARAMETER")
    fun update(gl: WebGL2RenderingContext, keysPressed: Set<String>, timeStamp: Double) {

        val timeAtThisFrame = Date().getTime()
        val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
        val t = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f
        timeAtLastFrame = timeAtThisFrame

        if (!keysPressed.isEmpty()) {
            frameCount = 0
        }
        frameCount = frameCount + 1
        camera.move(dt, keysPressed)

        if ("LEFT" in keysPressed) {
            gameObjects[0].position.x += dt * 10.0f
        }
        if ("RIGHT" in keysPressed) {
            gameObjects[0].position.x -= dt * 10.0f
        }
        if ("UP" in keysPressed) {
            gameObjects[0].position.y += dt * 10.0f
        }
        if ("DOWN" in keysPressed) {
            gameObjects[0].position.y -= dt * 10.0f
        }

        // clear the screen
        gl.clearColor(0.3f, 0.0f, 0.3f, 1.0f)
        gl.clearDepth(1.0f)

        val spawn = ArrayList<GameObject>()
        val killList = ArrayList<GameObject>()
        gameObjects.forEach {
            if (!it.move(dt, t, keysPressed, gameObjects, spawn)) {
                killList.add(it)
            }
        }

        lights[0].position.set(cos(t) * 5f, 5f, sin(t) * 5f, 1f)
        lightSphere.position.set(lights[0].position)

        spotLights[0].position.set(camera.position)
        spotLights[0].direction.set(camera.ahead)

        killList.forEach { gameObjects.remove(it) }
        spawn.forEach { gameObjects.add(it) }
        gameObjects.forEach { it.update() }

        mirrorCamera.position.set(camera.position.x, -camera.position.y, camera.position.z)
        mirrorCamera.pitch = -camera.pitch
        mirrorCamera.yaw = -camera.yaw
        mirrorCamera.roll = camera.roll
        mirrorCamera.update()

        framebuffers[0].bind(gl)
        gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
        backgroundMesh.draw(mirrorCamera)
        gameObjects.drop(1).forEach { it.draw(mirrorCamera, *lights, *spotLights) }

        defaultFramebuffer.bind(gl)
        gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
        backgroundMesh.draw(camera)
        gameObjects.forEach { it.draw(camera, *lights, *spotLights) }
    }
}
