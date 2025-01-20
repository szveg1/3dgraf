import org.w3c.dom.HTMLCanvasElement
import org.w3c.dom.HTMLDivElement
import org.khronos.webgl.WebGLRenderingContext as GL
import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint32Array
import vision.gears.webglmath.*
import kotlin.js.Date
import kotlin.math.*
import kotlin.random.*

class Scene (
  val gl : WebGL2RenderingContext,
  val overlay : HTMLDivElement,
  val canvas : HTMLCanvasElement) : UniformProvider("scene") {

  lateinit var defaultFramebuffer : DefaultFramebuffer
  lateinit var framebuffers : Array<Framebuffer>
  var frameCount = 0

  val vsQuad = Shader(gl, GL.VERTEX_SHADER, "quad-vs.glsl")
  val fsBackground = Shader(gl, GL.FRAGMENT_SHADER, "background-fs.glsl")

  val fsAverage = Shader(gl, GL.FRAGMENT_SHADER, "average-fs.glsl")

  val fsShow = Shader(gl, GL.FRAGMENT_SHADER, "show-fs.glsl")

  val quadGeometry = TexturedQuadGeometry(gl)
  val flipQuadGeometry = FlipQuadGeometry(gl)

  val framebufferCube = FramebufferCube(gl, 1, 512)

  val averageMesh = Mesh(Material(Program(gl, vsQuad, fsAverage)), flipQuadGeometry)

  val showMesh = Mesh(Material(Program(gl, vsQuad, fsShow)), flipQuadGeometry)

  val backgroundProgram = Program(gl, vsQuad, fsBackground)
  val backgroundMaterial = Material(backgroundProgram)
  val skyCubeTexture = TextureCube(gl,
      "media/posx512.jpg", "media/negx512.jpg",
      "media/posy512.jpg", "media/negy512.jpg",
      "media/posz512.jpg", "media/negz512.jpg"
    )
  init {
    backgroundMaterial["envTexture"]?.set( skyCubeTexture )
  }
  val backgroundMesh = Mesh(backgroundMaterial, quadGeometry)

  val jsonLoader = JsonLoader()
  val exhibitGeometries = jsonLoader.loadGeometries(gl, "media/slowpoke/Slowpoke.json")

  val vsTransform = Shader(gl, GL.VERTEX_SHADER, "transform-vs.glsl")
  val fsPhong = Shader(gl, GL.FRAGMENT_SHADER, "phong-fs.glsl")
  val phongProgram = Program(gl, vsTransform, fsPhong)
  val phongMaterial = Material(phongProgram).apply {
    this["kd"]?.set(Vec3(0.6f, 0.2f, 0.1f))
    this["ks"]?.set(Vec3(1f, 1f, 1f))
    this["shininess"]?.set(15f)
  }

  val fsEnvmapped = Shader(gl, GL.FRAGMENT_SHADER, "envmapped-fs.glsl")
  val envmappedProgram = Program(gl, vsTransform, fsEnvmapped)
  val envmappedMaterial = Material(envmappedProgram).apply {
    this["kr"]?.set(Vec3(0.4f, 0.4f, 0.3f))
    this["environment"]?.set(framebufferCube.targets[0])
  }

  val gameObjects = ArrayList<GameObject>()
  init {
    gameObjects.add(GameObject(*exhibitGeometries.map { Mesh(envmappedMaterial, it) }.toTypedArray()))
    val radius = 27f
    for (i in 0 until 5) {
      gameObjects += GameObject(*exhibitGeometries.map { Mesh(phongMaterial, it) }.toTypedArray()).apply{
        val phi = i.toFloat() / 5f * PI.toFloat() * 2f
        yaw = i.toFloat()
        position.set(
          cos(phi) * radius,
          -10f,
          sin(phi) * radius
        )
      }
    }
  }

  val lights = Array<Light>(1) { Light(it) }
  init{
    lights[0].position.set(1.0f, 1.0f, 1.0f, 0.0f).normalize()
    lights[0].powerDensity.set(5.0f, 5.0f, 5.0f)
  }

  val camera = PerspectiveCamera()
  val confusedCamera = PerspectiveCamera()  
  var focalDistance = 15f

  val cubeCameras = arrayOf(
    PerspectiveCamera().apply{ //+X
      fov = PI.toFloat()/2f
      yaw = -PI.toFloat()/2f
      roll = PI.toFloat()
      update()
    },
    PerspectiveCamera().apply{ //-X
      fov = PI.toFloat()/2f
      yaw = PI.toFloat()/2f
      roll = PI.toFloat()
      update()
    },
    PerspectiveCamera().apply{ //+Y
      fov = PI.toFloat()/2f
      pitch = PI.toFloat()/2f
      update()
    },
    PerspectiveCamera().apply{ //-Y
      fov = PI.toFloat()/2f
      pitch = -PI.toFloat()/2f
      update()
    },
    PerspectiveCamera().apply{ //+Z
      fov = PI.toFloat()/2f
      roll = PI.toFloat()
      yaw = PI.toFloat()
      update()
    },
    PerspectiveCamera().apply{ //-Z
      fov = PI.toFloat()/2f
      roll = PI.toFloat()
      update()
    },
  )

  val timeAtFirstFrame = Date().getTime()
  var timeAtLastFrame =  timeAtFirstFrame

  init{
    gl.enable(GL.DEPTH_TEST)
  }


  fun resize(gl : WebGL2RenderingContext, canvas : HTMLCanvasElement) {
    gl.viewport(0, 0, canvas.width, canvas.height)
    camera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())

    defaultFramebuffer = DefaultFramebuffer(canvas.width, canvas.height)
    framebuffers = arrayOf(
      Framebuffer(gl, 1, canvas.width, canvas.height, GL.RGBA32F, GL.RGBA, GL.FLOAT),
      Framebuffer(gl, 1, canvas.width, canvas.height, GL.RGBA32F, GL.RGBA, GL.FLOAT),
      Framebuffer(gl, 1, canvas.width, canvas.height, GL.RGBA32F, GL.RGBA, GL.FLOAT)
    )
  }

  @Suppress("UNUSED_PARAMETER")
  fun update(gl : WebGL2RenderingContext, keysPressed : Set<String>, timeStamp : Double) {

    val timeAtThisFrame = Date().getTime() 
    val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
    val t  = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f    
    timeAtLastFrame = timeAtThisFrame

    if(!keysPressed.isEmpty()){
      frameCount = 0
    }
    frameCount = frameCount+1
    camera.move(dt, keysPressed)

    if("LEFT" in keysPressed){
      gameObjects[0].position.x += dt * 10.0f
    }
    if("RIGHT" in keysPressed){
      gameObjects[0].position.x -= dt * 10.0f
    }
    if("UP" in keysPressed){
      gameObjects[0].position.y += dt * 10.0f
    }
    if("DOWN" in keysPressed){
      gameObjects[0].position.y -= dt * 10.0f
    }            

    // clear the screen
    gl.clearColor(0.3f, 0.0f, 0.3f, 1.0f)
    gl.clearDepth(1.0f)

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

    for(i in 0 until 6){
      cubeCameras[i].position.set(gameObjects[0].position)
      cubeCameras[i].update()
      framebufferCube.bind(gl, i)
      gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
      backgroundMesh.draw(cubeCameras[i])
      gameObjects.drop(1).forEach { it.draw(cubeCameras[i], *lights) }
    }

    framebuffers[0].bind(gl)
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
    confusedCamera.position.set(camera.position)
    confusedCamera.position += camera.right * Random.nextFloat()
    confusedCamera.position += camera.up * Random.nextFloat()
    confusedCamera.lookAt(
      camera.position + camera.ahead * focalDistance)

    backgroundMesh.draw(confusedCamera)
    gameObjects.forEach { it.draw( confusedCamera, *lights ) }

    framebuffers[2].bind(gl)
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
    averageMesh["currentFrame"]?.set(framebuffers[0].targets[0])
    averageMesh["prevFrame"]?.set(framebuffers[1].targets[0])
    averageMesh["weight"]?.set(1.0f/frameCount.toFloat())
    averageMesh.draw()

    defaultFramebuffer.bind(gl)
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)
    showMesh["averagedFrame"]?.set(framebuffers[2].targets[0])
    showMesh.draw()

    framebuffers[1] = framebuffers[2].also{
      framebuffers[2] = framebuffers[1]
    }
  }
}
