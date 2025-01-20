import org.w3c.dom.HTMLCanvasElement
import org.khronos.webgl.WebGLRenderingContext as GL
import vision.gears.webglmath.*
import kotlin.js.Date
import kotlin.math.*
import kotlin.random.Random

class Scene (
  val gl : WebGL2RenderingContext) : UniformProvider("scene") {

  val vsTrafo = Shader(gl, GL.VERTEX_SHADER, "trafo-vs.glsl")
  val fsTextured = Shader(gl, GL.FRAGMENT_SHADER, "textured-fs.glsl")
  val texturedProgram = Program(gl, vsTrafo, fsTextured)

  val quadGeometry = TexturedQuadGeometry(gl)

  val timeAtFirstFrame = Date().getTime()
  var timeAtLastFrame =  timeAtFirstFrame

  val raiderMaterial = Material(texturedProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl, "media/raider.png"))
    this["texOffset"]?.set(0.1f, 0.4f)
  }

  val asteroidMaterial = Material(texturedProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl, "media/asteroid.png"))
    this["texOffset"]?.set(0.1f, 0.4f)
  }

  val raiderMesh = Mesh(raiderMaterial, quadGeometry)
  val asteroidMesh = Mesh(asteroidMaterial, quadGeometry)
  val gameObjects = ArrayList<GameObject>()

  val camera = OrthoCamera()

  val avatar = GameObject(raiderMesh).apply{
    move = object : GameObject.Motion(this){
      override operator fun invoke(
        dt : Float,
        t : Float,
        keysPressed : Set<String>,
        interactors : ArrayList<GameObject>,
        spawn : ArrayList<GameObject>
      ) : Boolean {
        if("Q" in keysPressed) roll += 0.025f
        if("E" in keysPressed) roll -= 0.025f

        var velocity = Vec3(0.0f, 0.0f)
        if("W" in keysPressed) velocity.x = 1.0f
        if("A" in keysPressed) velocity.y = -1.0f
        if("S" in keysPressed) velocity.x = -1.0f
        if("D" in keysPressed) velocity.y = 1.0f

        velocity = (Vec4(velocity) * Mat4().rotate(roll, Vec3(0.0f, 0.0f, 1.0f))).xyz

        gameObject.position += (velocity * dt)
        camera.position.set(gameObject.position)
        return true
      }
    }
    scale.set(0.1f, 0.1f)
  }

  fun createAsteroid(mesh: Mesh): GameObject {
    return GameObject(mesh).apply {
      move = object : GameObject.Motion(this) {
        var velocity = Vec3(Random.nextFloat(), Random.nextFloat())
        override operator fun invoke(
          dt: Float,
          t: Float,
          keysPressed: Set<String>,
          interactors: ArrayList<GameObject>,
          spawn: ArrayList<GameObject>
        ): Boolean {
          gameObject.position += velocity * dt
          roll += 0.025f
          return true
        }
      }
      scale.set(0.1f, 0.1f)
      position.set(Random.nextDouble(-10.0, 10.0).toFloat(), Random.nextDouble(-10.0, 10.0).toFloat())
    }
  }

  init {
    gameObjects += avatar
    for(i in 1..100){
      gameObjects += createAsteroid(asteroidMesh)
    }
  }

  fun resize(gl : WebGL2RenderingContext, canvas : HTMLCanvasElement) {
    gl.viewport(0, 0, canvas.width, canvas.height)
    camera.setAspectRatio((canvas.width/canvas.height).toFloat())
  }

  @Suppress("UNUSED_PARAMETER")
  fun update(gl : WebGL2RenderingContext, keysPressed : Set<String>) {

    val timeAtThisFrame = Date().getTime() 
    val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
    val t  = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f    
    timeAtLastFrame = timeAtThisFrame

    gl.enable(GL.BLEND)
    gl.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA)

    // clear the screen
    gl.clearColor(0.2f, 0.0f, 0.5f, 1.0f)
    gl.clearDepth(1.0f)
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)

    gl.useProgram(texturedProgram.glProgram)

    val spawn = ArrayList<GameObject>()
    val killList = ArrayList<GameObject>()
    gameObjects.forEach {
      if(!it.move(dt, t, keysPressed, gameObjects, spawn)){
        killList.add(it)
      }
    }
    killList.forEach{ gameObjects.remove(it) }
    spawn.forEach{ gameObjects.add(it) }
    gameObjects.forEach { it.update() }
    gameObjects.forEach { it.draw(camera) }
    camera.update()
  }
}
