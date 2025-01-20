import org.w3c.dom.HTMLCanvasElement
import org.khronos.webgl.WebGLRenderingContext as GL
import vision.gears.webglmath.*
import kotlin.js.Date
import kotlin.math.exp
import kotlin.random.Random

class Scene (
  val gl : WebGL2RenderingContext) : UniformProvider("scene") {

  val vsTrafo = Shader(gl, GL.VERTEX_SHADER, "trafo-vs.glsl")
  val vsBackground = Shader(gl, GL.VERTEX_SHADER, "background-vs.glsl")
  val fsTextured = Shader(gl, GL.FRAGMENT_SHADER, "textured-fs.glsl")

  val texturedProgram = Program(gl, vsTrafo, fsTextured)
  val backgroundProgram = Program(gl, vsBackground, fsTextured)

  val quadGeometry = TexturedQuadGeometry(gl)

  val timeAtFirstFrame = Date().getTime()
  var timeAtLastFrame =  timeAtFirstFrame

  val raiderMaterial = Material(texturedProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl, "media/raider.png"))
  }

  val asteroidMaterial = Material(texturedProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl, "media/asteroid.png"))
  }

  val backgroundMaterial = Material(backgroundProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl, "media/nebula.jpg")
    )
  }

  val bulletMaterial = Material(texturedProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl,"media/laserBullet.png")
    )
  }

  val explosionMaterial = Material(texturedProgram).apply{
    this["colorTexture"]?.set(
      Texture2D(gl,"media/boom.png")
    )
  }

  val raiderMesh = Mesh(raiderMaterial, quadGeometry)
  val asteroidMesh = Mesh(asteroidMaterial, quadGeometry)
  val backgroundMesh = Mesh(backgroundMaterial, quadGeometry)
  val bulletMesh = Mesh(bulletMaterial, quadGeometry)
  val explosionMesh = Mesh(explosionMaterial, quadGeometry)

  val gameObjects = ArrayList<GameObject>()

  val camera = OrthoCamera()

  val airResistance = 5.0f

  val background = GameObject(backgroundMesh).apply{
    radius = -999f
  }

  fun isCollision(obj1: GameObject, obj2: GameObject) : Boolean {
    val distance = (obj1.position - obj2.position).length()
    return distance < (obj1.radius + obj2.radius)
  }

  fun createExplosion(destroyedObject: GameObject) : GameObject {
    return GameObject(explosionMesh).apply{
      var phase = 0
      scale.set(0.3f, 0.3f)
      position.set(destroyedObject.position)
      /// To make sure you cant explode from the explosion itself
      radius = -999f

      val texMatrix = Mat4().set().scale(1.0f/6.0f, 1.0f/6.0f)
      this["texMatrix"]?.set(texMatrix)

      move = object : GameObject.Motion(this){
        override operator fun invoke(
          dt : Float,
          t : Float,
          keysPressed : Set<String>,
          interactors : ArrayList<GameObject>,
          spawn : ArrayList<GameObject>
        ) : Boolean {
          while (phase < 36){
            val x = (phase % 6) / 6.0f
            val y = (phase / 6) / 6.0f
            texMatrix.set().scale(1.0f/6.0f, 1.0f/6.0f).translate(x, y)
            gameObject["texMatrix"]?.set(texMatrix)
            phase++
            return true
            }
          return false
          }
        }
      }
    }

  fun fireBullet(shooter: GameObject) : GameObject {
    return GameObject(bulletMesh).apply{
      roll = shooter.roll
      val offset = (Vec4(0.1f) * Mat4().rotate(roll, Vec3(0.0f, 0.0f, 1.0f))).xyz
      position.set(shooter.position + offset)
      scale.set(0.025f, 0.025f)
      radius = 0.025f
      move = object : GameObject.Motion(this){
        override operator fun invoke(
          dt : Float,
          t : Float,
          keysPressed : Set<String>,
          interactors : ArrayList<GameObject>,
          spawn : ArrayList<GameObject>
        ) : Boolean {
          val velocity = (Vec4(5.0f) * Mat4().rotate(roll, Vec3(0.0f, 0.0f, 1.0f))).xyz

          gameObject.position += (velocity * dt)

          for(interactor in interactors){
            if(interactor != shooter && interactor != gameObject && isCollision(interactor, gameObject)){
              spawn += createExplosion(interactor)
              interactors.remove(interactor)
              return false
            }
          }

          return true
        }
      }
    }
  }

  val avatar = GameObject(raiderMesh).apply{
    val mass = 5.0f
    val acceleration = Vec3()
    val velocity = Vec3()

    var timeToNextShot = 0.0f
    val cooldown = 0.5f
    scale.set(0.1f, 0.1f)
    radius = 0.1f

    val firedBullets = ArrayList<GameObject>()

    move = object : GameObject.Motion(this) {
      override operator fun invoke(
        dt: Float,
        t: Float,
        keysPressed: Set<String>,
        interactors: ArrayList<GameObject>,
        spawn: ArrayList<GameObject>
      ): Boolean {
        if ("Q" in keysPressed) roll += 0.025f
        if ("E" in keysPressed) roll -= 0.025f

        acceleration.set()

        if ("W" in keysPressed) acceleration += mass * Vec3(0.5f, 0.0f, 0.0f)
        if ("A" in keysPressed) acceleration += mass * Vec3(0.0f, 0.5f, 0.0f)
        if ("S" in keysPressed) acceleration += mass * Vec3(-0.5f, 0.0f, 0.0f)
        if ("D" in keysPressed) acceleration += mass * Vec3(0.0f, -0.5f, 0.0f)

        acceleration.set(
          (Vec4(acceleration) * Mat4().rotate(roll, Vec3(0.0f,0.0f,1.0f))).xyz
        )

        velocity.plusAssign(acceleration * dt)
        velocity.timesAssign(exp(-dt * airResistance/mass))

        gameObject.position += (velocity * dt)

        if ("SPACE" in keysPressed && timeToNextShot < 0) {
          val bullet = fireBullet(gameObject)
          firedBullets += bullet
          spawn += bullet
          timeToNextShot = cooldown
        }
        timeToNextShot -= dt

        for(interactor in interactors){
          if(interactor != gameObject && !firedBullets.contains(interactor) && isCollision(interactor, gameObject)){
            spawn += createExplosion(gameObject)
            return false
          }
        }

        camera.position.set(gameObject.position)
        return true
      }
    }
  }

  fun createEnemy(mesh: Mesh): GameObject {
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
    }
  }

  fun createAsteroid(mesh: Mesh): GameObject {
    return GameObject(mesh).apply {
      scale.set(0.1f, 0.1f)
      val spawnRadius = 5.0
      val randomSpawn = Vec3(Random.nextDouble(-spawnRadius, spawnRadius).toFloat(), Random.nextDouble(-spawnRadius, spawnRadius).toFloat())
      position.set(avatar.position + randomSpawn)
      radius = 0.1f
      val randomRoll = Random.nextDouble(-0.025, 0.025).toFloat()

      val velocity = Vec3(/*x*/Random.nextDouble(-1.0, 1.0).toFloat(),
                          /*y*/Random.nextDouble(-1.0, 1.0).toFloat())

      move = object : GameObject.Motion(this) {
        override operator fun invoke(
          dt: Float,
          t: Float,
          keysPressed: Set<String>,
          interactors: ArrayList<GameObject>,
          spawn: ArrayList<GameObject>
        ): Boolean {
          gameObject.position += velocity * dt
          roll += randomRoll
          // Keep the asteroids in a sensible distance from the player
          val outsideRadius = (gameObject.position - avatar.position).length() > spawnRadius.toFloat()
          var isDestroyed = false
          for(interactor in interactors){
            if(interactor != gameObject && isCollision(gameObject, interactor)) {
              interactors.remove(interactor)
              isDestroyed = true
            }
          }
          if(outsideRadius || isDestroyed){
            spawn += createExplosion(gameObject)
            spawn += createAsteroid(asteroidMesh)
            return false
          }
          return true
        }
      }
    }
  }

  fun destroyAndReplaceAsteroid(asteroid: GameObject){
    gameObjects.remove(asteroid)
    gameObjects += createAsteroid(asteroidMesh)
  }

  init {
    gameObjects += background
    gameObjects += avatar
    for(i in 1..10){
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
