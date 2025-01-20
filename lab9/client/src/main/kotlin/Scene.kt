import org.w3c.dom.HTMLCanvasElement
import org.khronos.webgl.WebGLRenderingContext as GL
import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint32Array
import vision.gears.webglmath.UniformProvider
import vision.gears.webglmath.Geometry
import vision.gears.webglmath.Vec3
import vision.gears.webglmath.Mat4
import kotlin.js.Date

class Scene (
  val gl : WebGL2RenderingContext) : UniformProvider("scene") {

  val vsTrafo = Shader(gl, GL.VERTEX_SHADER, "trafo-vs.glsl")
  val fsTextured = Shader(gl, GL.FRAGMENT_SHADER, "textured-fs.glsl")
  val fsEnvmapped = Shader(gl, GL.FRAGMENT_SHADER, "envmapped-fs.glsl")
  val fsMaxBlinn = Shader(gl, GL.FRAGMENT_SHADER, "maxblinn-fs.glsl")
  val texturedProgram = Program(gl, vsTrafo, fsTextured)
  val envmappedProgram = Program(gl, vsTrafo, fsEnvmapped)
  val maxBlinnProgram = Program(gl, vsTrafo, fsMaxBlinn)

  val vsQuad = Shader(gl, GL.VERTEX_SHADER, "quad-vs.glsl")
  val fsRaycast = Shader(gl, GL.FRAGMENT_SHADER, "raycast-fs.glsl")

  val quadGeometry = TexturedQuadGeometry(gl)

  val raycastProgram = Program(gl, vsQuad, fsRaycast)
  val raycastMaterial = Material(raycastProgram)
  val skyCubeTexture = TextureCube(gl,
      "media/posx512.jpg", "media/negx512.jpg",
      "media/posy512.jpg", "media/negy512.jpg",
      "media/posz512.jpg", "media/negz512.jpg"
    )
  init {
    raycastMaterial["envTexture"]?.set( skyCubeTexture )
    raycastMaterial["noiseTexture"]?.set( Texture2D(gl, "media/noise.png") )    
  }
  val raycastMesh = Mesh(raycastMaterial, quadGeometry)

  val gameObjects = ArrayList<GameObject>()

  val lights = Array<Light>(1) { Light(it, *Program.all) }
  init{
    lights[0].position.set(-1.0f, 1.0f, -1.0f, 0.0f).normalize()
    lights[0].powerDensity.set(1.0f, 1.0f, 1.0f)
  }

  val camera = PerspectiveCamera(*Program.all)

  val timeAtFirstFrame = Date().getTime()
  var timeAtLastFrame =  timeAtFirstFrame

  init{
    gl.enable(GL.DEPTH_TEST)
    addComponentsAndGatherUniforms(*Program.all)
  }

  fun resize(gl : WebGL2RenderingContext, canvas : HTMLCanvasElement) {
    gl.viewport(0, 0, canvas.width, canvas.height)
    camera.setAspectRatio(canvas.width.toFloat() / canvas.height.toFloat())
  }

  @Suppress("UNUSED_PARAMETER")
  fun update(gl : WebGL2RenderingContext, keysPressed : Set<String>) {

    val timeAtThisFrame = Date().getTime() 
    val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
    val t  = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f    
    timeAtLastFrame = timeAtThisFrame

    camera.move(dt, keysPressed)

    // clear the screen
    gl.clearColor(0.3f, 0.0f, 0.3f, 1.0f)
    gl.clearDepth(1.0f)
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)

    //gameObjects.forEach { it.move(t, dt, keysPressed, gameObjects) }
    //gameObjects.forEach { it.update() }

    raycastMesh.draw(camera, this)
    //gameObjects.forEach { it.draw( camera, *lights ) }

  }
}
