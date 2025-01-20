import org.w3c.dom.HTMLCanvasElement
import org.w3c.dom.HTMLDivElement
import org.khronos.webgl.WebGLRenderingContext as GL
import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint32Array
import vision.gears.webglmath.UniformProvider
import vision.gears.webglmath.Geometry
import vision.gears.webglmath.Vec3
import vision.gears.webglmath.Mat4
import kotlin.js.Date

import kotlinx.serialization.*
import kotlinx.serialization.json.*
import GameState

class Scene(
    val gl: WebGL2RenderingContext, var playerId: Int, var playerName: String
) : UniformProvider("scene") {

    val json = Json { ignoreUnknownKeys = true }

    fun applyServerState(serializedServerState: String) {
        gameState.applyServerState(
            json.decodeFromString(
                GameState.serializer(),
                serializedServerState
            )
        )
    }

    val vsTrafo = Shader(gl, GL.VERTEX_SHADER, "trafo-vs.glsl")
    val fsTextured = Shader(gl, GL.FRAGMENT_SHADER, "textured-fs.glsl")
    val texturedProgram = Program(gl, vsTrafo, fsTextured)
    val vsBackground = Shader(gl, GL.VERTEX_SHADER, "background-vs.glsl")

    val quadGeometry = TexturedQuadGeometry(gl)

    val backgroundProgram = Program(gl, vsBackground, fsTextured)
    val backgroundMaterial = Material(backgroundProgram).apply {
        this["colorTexture"]?.set(Texture2D(gl, "media/nebula.jpg"))
        this["texScale"]?.set(1.0f, 1.0f)
    }
    val backgroundMesh = Mesh(backgroundMaterial, quadGeometry)

    val meshes = HashMap<String, Mesh>()

    fun getMesh(textureFileName: String, animTileSizeX: Float = 1.0f, animTileSizeY: Float = 1.0f): Mesh {
        return meshes.getOrPut(textureFileName) {
            val material = Material(texturedProgram).apply {
                this["colorTexture"]?.set(Texture2D(gl, "media/" + textureFileName + ".png"))
                this["texScale"]?.set(animTileSizeX, animTileSizeY)
            }
            val mesh = Mesh(material, quadGeometry)
            meshes[textureFileName] = mesh
            return mesh
        }
    }

    val gameState = GameState()
    val gameObjects = HashMap<Int, GameObject>()

    val avatar = GameObject(getMesh("fighter"))

    init {
        getMesh("orb", 0.2f, 0.2f)
        gameState.addObject(playerId, "fighter", playerName)
        gameObjects[playerId] = avatar
        avatar.label = playerName
    }

    val camera = OrthoCamera(*Program.all)

    val timeAtFirstFrame = Date().getTime()
    var timeAtLastFrame = timeAtFirstFrame

    init {
        gl.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA)
        gl.enable(GL.BLEND)
        addComponentsAndGatherUniforms(*Program.all)
    }

    fun resize(gl: WebGL2RenderingContext, canvas: HTMLCanvasElement) {
        gl.viewport(0, 0, canvas.width, canvas.height)
        camera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())
    }

    @Suppress("UNUSED_PARAMETER")
    fun update(gl: WebGL2RenderingContext, overlay: HTMLDivElement, keysPressed: Set<String>) {

        val timeAtThisFrame = Date().getTime()
        val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
        val t = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f
        timeAtLastFrame = timeAtThisFrame

        camera.position.set(avatar.position)
        camera.updateViewProjMatrix()

        // clear the screen
        gl.clearColor(0.3f, 0.0f, 0.3f, 1.0f)
        gl.clearDepth(1.0f)
        gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)

        overlay.innerHTML = ""

        gameState.update(dt, t)
        gameState.objects.forEach { (netId: Int, netObj: NetObject) ->
            gameObjects.getOrPut(netId) {
                GameObject(getMesh(netObj.meshName))
            }.interpolate(
                dt = dt,
                targetPosition = netObj.position,
                targetRoll = netObj.roll)
//        .apply{
//        position.set(netObj.position)
//        roll = netObj.roll
//        scale.set(netObj.collisionRadius, netObj.collisionRadius, 1.0f)
//        label = netObj.label
//      }
        }

        gameObjects.keys.removeAll {
            gameState.objects[it] == null
        }
        gameObjects.forEach { (netId, gameObject) -> gameObject.move(dt, t) }
        gameObjects.forEach { (netId, gameObject) -> gameObject.update() }

        backgroundMesh.draw(camera)
        gameObjects.forEach { (netId, gameObject) -> gameObject.draw(camera) }
        gameObjects.forEach { (netId, gameObject) ->
            overlay.innerHTML += gameObject.drawHTML(
                camera.viewProjMatrix,
                overlay.offsetParent?.clientWidth ?: 100,
                overlay.offsetParent?.clientHeight ?: 100
            )
        }

    }

    fun onKeyDown(key: String) {
        gameState.onKeyDown(playerId, key)
    }

    fun onKeyUp(key: String) {
        gameState.onKeyUp(playerId, key)
    }

}
