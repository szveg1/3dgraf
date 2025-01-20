import org.w3c.dom.HTMLCanvasElement
import org.w3c.dom.HTMLDivElement
import vision.gears.webglmath.Mat4
import vision.gears.webglmath.UniformProvider
import vision.gears.webglmath.Vec3
import vision.gears.webglmath.Vec4
import kotlin.js.Date
import kotlin.math.PI
import kotlin.math.abs
import kotlin.math.exp
import kotlin.math.tan
import org.khronos.webgl.WebGLRenderingContext as GL

class Scene(
    val gl: WebGL2RenderingContext, val overlay: HTMLDivElement, val canvas: HTMLCanvasElement
) : UniformProvider("scene") {

    lateinit var defaultFramebuffer: DefaultFramebuffer
    var frameCount = 0

    val vsQuad = Shader(gl, GL.VERTEX_SHADER, "quad-vs.glsl")
    val vsTransform = Shader(gl, GL.VERTEX_SHADER, "transform-vs.glsl")
    val vsProjected = Shader(gl, GL.VERTEX_SHADER, "plane-projected-vs.glsl")

    val fsBackground = Shader(gl, GL.FRAGMENT_SHADER, "background-fs.glsl")
    val fsTextured = Shader(gl, GL.FRAGMENT_SHADER, "textured-fs.glsl")
    val fsInf = Shader(gl, GL.FRAGMENT_SHADER, "infiniteplane-fs.glsl")
    val fsProjected = Shader(gl, GL.FRAGMENT_SHADER, "plane-projected-fs.glsl")

    val backgroundProgram = Program(gl, vsQuad, fsBackground)
    val texturedProgram = Program(gl, vsTransform, fsTextured)
    val infProgram = Program(gl, vsTransform, fsInf)
    val projectedProgram = Program(gl, vsProjected, fsProjected)

    val quadGeometry = TexturedQuadGeometry(gl)
    val infinitePlaneGeometry = InfinitePlaneGeometry(gl)

    val skyCubeTexture = TextureCube(
        gl,
        "media/cubemap/px.png",
        "media/cubemap/nx.png",
        "media/cubemap/py.png",
        "media/cubemap/ny.png",
        "media/cubemap/pz.png",
        "media/cubemap/nz.png"
    )

    val shadowMaterial = Material(projectedProgram)

    val backgroundMesh = Mesh(
        Material(backgroundProgram).apply {
            this["envTexture"]?.set(skyCubeTexture)
        }, quadGeometry
    )

    val groundMesh = Mesh(
        Material(infProgram).apply {
            this["colorTexture"]?.set(
                Texture2D(gl, "media/grass.png")
            )
        }, infinitePlaneGeometry
    )

    val jsonLoader = JsonLoader()

    val chevyMeshes = jsonLoader.loadMeshes(gl, "media/json/chevy/chassis.json", Material(texturedProgram).apply {
        this["colorTexture"]?.set(
            Texture2D(gl, "media/json/chevy/chevy.png")
        )
    })
    val wheelMeshes = jsonLoader.loadMeshes(gl, "media/json/chevy/wheel.json", Material(texturedProgram).apply {
        this["colorTexture"]?.set(
            Texture2D(gl, "media/json/chevy/chevy.png")
        )
    })


    val chevy = GameObject(*chevyMeshes).apply {
        val wheelbase = 3.302f
        val invMass = 1 / 2600f
        val momentum = Vec3()
        val dragCoefficient = 200f
        val brakeCoefficient = 1500f

        val force = Vec3()
        var steeringAngle = 0f


        move = object : GameObject.Motion(this) {
            override operator fun invoke(
                dt: Float,
                t: Float,
                keysPressed: Set<String>,
                interactors: ArrayList<GameObject>,
                spawn: ArrayList<GameObject>
            ): Boolean {
                val targetForce = when {
                    "UP" in keysPressed -> Vec3(0f, 0f, 30000f)
                    "DOWN" in keysPressed -> Vec3(0f, 0f, -25000f)
                    else -> Vec3()
                }

                force.plusAssign((targetForce - force) * dt)


                val targetSteeringAngle = when {
                    "LEFT" in keysPressed -> 0.07f
                    "RIGHT" in keysPressed -> -0.07f
                    else -> 0f
                }

                steeringAngle += (targetSteeringAngle - steeringAngle) * dt

                momentum.plusAssign(force * dt)
                momentum.timesAssign(exp(-dt * dragCoefficient * invMass))
                if ("SPACE" in keysPressed) momentum.timesAssign(exp(-dt * brakeCoefficient * invMass))

                velocity.set(momentum * invMass)

                val turnRadius = if (abs(steeringAngle) > 0.0001f) {
                    wheelbase / tan(steeringAngle)
                } else {
                    Float.POSITIVE_INFINITY
                }

                if (turnRadius != Float.POSITIVE_INFINITY) {
                    val angularVelocity = velocity.z / turnRadius
                    yaw += angularVelocity * dt
                }

                velocity.set((Vec4(velocity) * Mat4().rotate(yaw, 0f, 1f, 0f)).xyz)
                position += velocity * dt

                return true
            }
        }
    }

    val wheelDiameter = 2f
    val wheelFrontLeft = GameObject(*wheelMeshes).apply {
        parent = chevy
        position.set(6.5f, -3.0f, 14.0f)
        move = object : GameObject.Motion(this) {
            override operator fun invoke(
                dt: Float,
                t: Float,
                keysPressed: Set<String>,
                interactors: ArrayList<GameObject>,
                spawn: ArrayList<GameObject>
            ): Boolean {
                val targetYaw = when {
                    "LEFT" in keysPressed -> 0.7f
                    "RIGHT" in keysPressed -> -0.7f
                    else -> 0f
                }

                yaw += (targetYaw - yaw) * dt

                val parentVelocity = parent!!.velocity
                val angularVelocity = parentVelocity / wheelDiameter
                val parentVelocityRotatedBack = (Vec4(parentVelocity) * Mat4().rotate(-parent!!.yaw, 0f, 1f, 0f)).xyz

                pitch += if(parentVelocityRotatedBack.z >= 0) angularVelocity.length() * dt else -angularVelocity.length() * dt

                return true
            }
        }
    }
    val wheelFrontRight = GameObject(*wheelMeshes).apply {
        parent = chevy
        position.set(-6.5f, -3.0f, 14.0f)
        scale.set(-1.0f, 1.0f, -1.0f)
        move = object : GameObject.Motion(this) {
            override operator fun invoke(
                dt: Float,
                t: Float,
                keysPressed: Set<String>,
                interactors: ArrayList<GameObject>,
                spawn: ArrayList<GameObject>
            ): Boolean {
                val targetYaw = when {
                    "LEFT" in keysPressed -> 0.7f
                    "RIGHT" in keysPressed -> -0.7f
                    else -> 0f
                }
                yaw += (targetYaw - yaw) * dt

                val parentVelocity = parent!!.velocity
                val angularVelocity = parentVelocity / wheelDiameter
                val parentVelocityRotatedBack = (Vec4(parentVelocity) * Mat4().rotate(-parent!!.yaw, 0f, 1f, 0f)).xyz

                pitch += if(parentVelocityRotatedBack.z >= 0) angularVelocity.length() * dt else -angularVelocity.length() * dt

                return true
            }
        }
    }
    val wheelBackLeft = GameObject(*wheelMeshes).apply {
        parent = chevy
        position.set(6.5f, -3.0f, -11.0f)
        move = object : GameObject.Motion(this) {
            override operator fun invoke(
                dt: Float,
                t: Float,
                keysPressed: Set<String>,
                interactors: ArrayList<GameObject>,
                spawn: ArrayList<GameObject>
            ): Boolean {
                val parentVelocity = parent!!.velocity
                val angularVelocity = parentVelocity / wheelDiameter
                val parentVelocityRotatedBack = (Vec4(parentVelocity) * Mat4().rotate(-parent!!.yaw, 0f, 1f, 0f)).xyz

                pitch += if(parentVelocityRotatedBack.z >= 0) angularVelocity.length() * dt else -angularVelocity.length() * dt

                return true
            }
        }
    }
    val wheelBackRight = GameObject(*wheelMeshes).apply {
        parent = chevy
        position.set(-6.5f, -3.0f, -11.0f)
        scale.set(-1.0f, 1.0f, -1.0f)
        move = object : GameObject.Motion(this) {
            override operator fun invoke(
                dt: Float,
                t: Float,
                keysPressed: Set<String>,
                interactors: ArrayList<GameObject>,
                spawn: ArrayList<GameObject>
            ): Boolean {
                val parentVelocity = parent!!.velocity
                val angularVelocity = parentVelocity / wheelDiameter
                val parentVelocityRotatedBack = (Vec4(parentVelocity) * Mat4().rotate(-parent!!.yaw, 0f, 1f, 0f)).xyz

                pitch += if(parentVelocityRotatedBack.z >= 0) angularVelocity.length() * dt else -angularVelocity.length() * dt

                return true
            }
        }
    }
    val ground = GameObject(groundMesh).apply {
        pitch = PI.toFloat() / 2f
        position.set(0f, -6f, 0f)
    }

    val gameObjects = ArrayList<GameObject>()

    init {
        gameObjects += ground
        gameObjects += chevy
        gameObjects += wheelFrontLeft
        gameObjects += wheelFrontRight
        gameObjects += wheelBackLeft
        gameObjects += wheelBackRight
    }

    val lights = Array<Light>(1) { Light(it) }

    init {
        lights[0].position.set(0f, 1f, -1f, 0f).normalize()
        lights[0].powerDensity.set(1f, 1f, 1f)
    }

    val lightDir = lights[0].position

    init {
        addComponentsAndGatherUniforms(shadowMaterial)
        this["shadowMatrix"]?.set(
            Mat4(
                1f, 0f, 0f, 0f,
                0f, 0f, 0f, 0f,
                0f, 0f, 1f, 0f,
                lightDir.x, lightDir.y, lightDir.z, 1f
            ).translate(0f,ground.position.y - 0.7f, 0f)
        )
    }

    val camera = PerspectiveCamera()
    val heliCam = HeliCam().apply{
        position = chevy.position + Vec3(0f, 10f, -30f)
    }

    val timeAtFirstFrame = Date().getTime()
    var timeAtLastFrame = timeAtFirstFrame

    init {
        gl.enable(GL.DEPTH_TEST)
    }


    fun resize(gl: WebGL2RenderingContext, canvas: HTMLCanvasElement) {
        gl.viewport(0, 0, canvas.width, canvas.height)
        camera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())

        defaultFramebuffer = DefaultFramebuffer(canvas.width, canvas.height)
    }


    @Suppress("UNUSED_PARAMETER")
    fun update(gl: WebGL2RenderingContext, keysPressed: Set<String>, timeStamp: Double) {
        val timeAtThisFrame = Date().getTime()
        val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
        val t = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f
        timeAtLastFrame = timeAtThisFrame

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

        heliCam.update(dt, chevy.position + Vec3(0f, 10f, 0f), chevy.velocity)
        camera.position.set(heliCam.position)
        camera.lookAt(chevy.position)

        killList.forEach { gameObjects.remove(it) }
        spawn.forEach { gameObjects.add(it) }
        gameObjects.forEach { it.update() }

        defaultFramebuffer.bind(gl)
        gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
        backgroundMesh.draw(camera)
        gameObjects.forEach {
            it.draw(camera, *lights)
            if (it != ground)
                it.using(shadowMaterial).draw(this, camera)
        }
    }
}
