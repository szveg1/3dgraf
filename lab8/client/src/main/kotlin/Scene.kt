import org.khronos.webgl.Uint32Array
import org.w3c.dom.HTMLCanvasElement
import vision.gears.webglmath.Geometry
import vision.gears.webglmath.UniformProvider
import kotlin.js.Date
import org.khronos.webgl.WebGLRenderingContext as GL

class Scene(
    val gl: WebGL2RenderingContext
) : UniformProvider("scene") {

    val vsTrafo = Shader(gl, GL.VERTEX_SHADER, "trafo-vs.glsl")
    val fsTextured = Shader(gl, GL.FRAGMENT_SHADER, "textured-fs.glsl")
    val fsEnvmapped = Shader(gl, GL.FRAGMENT_SHADER, "envmapped-fs.glsl")
    val fsMaxBlinn = Shader(gl, GL.FRAGMENT_SHADER, "maxblinn-fs.glsl")
    val texturedProgram = Program(gl, vsTrafo, fsTextured)
    val envmappedProgram = Program(gl, vsTrafo, fsEnvmapped)
    val maxBlinnProgram = Program(gl, vsTrafo, fsMaxBlinn)

    val vsQuad = Shader(gl, GL.VERTEX_SHADER, "quad-vs.glsl")
    val fsBackground = Shader(gl, GL.FRAGMENT_SHADER, "background-fs.glsl")

    val quadGeometry = TexturedQuadGeometry(gl)

    val backgroundProgram = Program(gl, vsQuad, fsBackground)
    val backgroundMaterial = Material(backgroundProgram)
    val skyCubeTexture = TextureCube(
        gl,
        "media/posx512.jpg", "media/negx512.jpg",
        "media/posy512.jpg", "media/negy512.jpg",
        "media/posz512.jpg", "media/negz512.jpg"
    )

    init {
        backgroundMaterial["envTexture"]?.set(skyCubeTexture)
    }

    val backgroundMesh = Mesh(backgroundMaterial, quadGeometry)

    val fsInterpolation = Shader(gl, GL.FRAGMENT_SHADER, "interpolation-fs.glsl")
    val fsChain = Shader(gl, GL.FRAGMENT_SHADER, "chain-fs.glsl")
    val vsSkinning = Shader(gl, GL.VERTEX_SHADER, "skinning-vs.glsl")

    val interpolationProgram = Program(gl, vsQuad, fsInterpolation)
    val interpolationMaterial = Material(interpolationProgram)
    val interpolationQuad = Mesh(interpolationMaterial, quadGeometry)

    val chainProgram = Program(gl, vsQuad, fsChain)
    val chainMaterial = Material(chainProgram)
    val chainQuad = Mesh(chainMaterial, quadGeometry)

    val skinningProgram = Program(gl, vsSkinning, fsMaxBlinn, Program.PNTIW)
    val skinningMaterial = Material(skinningProgram)

    init {
        skinningMaterial["colorTexture"]?.set(Texture2D(gl, "media/mrem.jpg"))
    }

    val jsonLoader = JsonLoader()
    val riggedGeometry = jsonLoader.loadRigged(gl, "media/mrem.json")
    val skinnedMesh = MultiMesh(arrayOf(skinningMaterial),
        Array<Geometry>(riggedGeometry.size) { riggedGeometry[it] })

    val gameObjects = ArrayList<GameObject>()

    init {
        val animatedObject = GameObject(skinnedMesh)
        animatedObject.position.set(20.0f)
        gameObjects.add(animatedObject)
    }

    lateinit var defaultFramebuffer: DefaultFramebuffer

    //LABTODO
    val zombieGeom = riggedGeometry[0]
    val animation = Animation(
        gl,
        "./media/thriller_part_3.json",
        zombieGeom
    )
    val nNodes = animation.nNodes.toInt()

    val riggingTexture = DataTexture(gl, zombieGeom.rigging.storage, zombieGeom.nBones * 2)
    val skeletonTexture = DataTexture(
        gl,
        Uint32Array(animation.skeleton.buffer),
        zombieGeom.nBones,
        1,
        GL.RGBA32UI,
        GL.RGBA_INTEGER,
        GL.UNSIGNED_INT
    )
    val keyTexture = DataTexture(gl, animation.keys.storage, animation.nKeys * 2, nNodes)


    val nodeFramebuffer = Framebuffer(gl, 2, 1, nNodes, GL.RGBA32F, GL.RGBA, GL.FLOAT)
    val boneFramebuffer = Framebuffer(gl, 2, animation.geometry.nBones, 1, GL.RGBA32F, GL.RGBA, GL.FLOAT)

    init {
        interpolationQuad["keyTexture"]?.set(keyTexture)
        interpolationQuad["nNodes"]?.set(nNodes.toFloat())
        interpolationQuad["nKeys"]?.set(animation.nKeys.toFloat())

        chainQuad["skeletonTexture"]?.set(skeletonTexture)
        chainQuad["riggingTexture"]?.set(riggingTexture)
        chainQuad["nodeQTexture"]?.set(nodeFramebuffer.targets[0])
        chainQuad["nodeTTexture"]?.set(nodeFramebuffer.targets[1])
        chainQuad["nNodes"]?.set(nNodes.toFloat())
        chainQuad["nBones"]?.set(animation.geometry.nBones.toFloat())

        skinnedMesh["boneQTexture"]?.set(boneFramebuffer.targets[0])
        skinnedMesh["boneTTexture"]?.set(boneFramebuffer.targets[1])
        skinnedMesh["nBones"]?.set(animation.geometry.nBones.toFloat())
    }


    val lights = Array<Light>(1) { Light(it, *Program.all) }

    init {
        lights[0].position.set(1.0f, 1.0f, 1.0f, 0.0f).normalize()
        lights[0].powerDensity.set(1.0f, 1.0f, 1.0f)
    }

    val camera = PerspectiveCamera(*Program.all)

    val timeAtFirstFrame = Date().getTime()
    var timeAtLastFrame = timeAtFirstFrame

    init {
        camera.position.z += 500.0f
        gl.enable(GL.DEPTH_TEST)
        addComponentsAndGatherUniforms(*Program.all)
    }


    fun resize(gl: WebGL2RenderingContext, canvas: HTMLCanvasElement) {
        gl.viewport(0, 0, canvas.width, canvas.height)
        camera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())

        defaultFramebuffer = DefaultFramebuffer(canvas.width, canvas.height)
    }

    @Suppress("UNUSED_PARAMETER")
    fun update(gl: WebGL2RenderingContext, keysPressed: Set<String>) {

        val timeAtThisFrame = Date().getTime()
        val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
        val t = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f
        timeAtLastFrame = timeAtThisFrame

        camera.move(dt, keysPressed)
        interpolationQuad["animationTime"]?.set(t)

        gl.disable(GL.DEPTH_TEST)
        nodeFramebuffer.bind(gl)
        interpolationQuad.draw(this)

        boneFramebuffer.bind(gl)
        chainQuad.draw(this)

        gl.enable(GL.DEPTH_TEST)
        defaultFramebuffer.bind(gl)

        // clear the screen
        gl.clearColor(0.3f, 0.0f, 0.3f, 1.0f)
        gl.clearDepth(1.0f)
        gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)

        val spawn = ArrayList<GameObject>()
        val killList = ArrayList<GameObject>()
        gameObjects.forEach {
            if (!it.move(t, dt, keysPressed, gameObjects, spawn)) {
                killList.add(it)
            }
        }
        killList.forEach { gameObjects.remove(it) }
        spawn.forEach { gameObjects.add(it) }
        gameObjects.forEach { it.update() }

        backgroundMesh.draw(camera)
        gameObjects.forEach { it.draw(camera, *lights) }

    }
}
