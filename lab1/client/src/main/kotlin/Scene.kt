import org.khronos.webgl.Float32Array
import org.w3c.dom.HTMLCanvasElement
import vision.gears.webglmath.Mat4
import vision.gears.webglmath.Vec2
import vision.gears.webglmath.Vec3
import kotlin.js.Date
import org.khronos.webgl.WebGLRenderingContext as GL //# GL# we need this for the constants declared ˙HUN˙ a constansok miatt kell

class Scene (
  val gl : WebGL2RenderingContext){

  val vsIdle = Shader(gl, GL.VERTEX_SHADER, "idle-vs.glsl")
  val fsSolid = Shader(gl, GL.FRAGMENT_SHADER, "solid-fs.glsl")
  val solidProgram = Program(gl, vsIdle, fsSolid)
  val quadGeometry = TexturedQuadGeometry(gl)

  val triangleGeometry = TexturedTriangleGeometry(gl)
  val fsGarish = Shader(gl,GL.FRAGMENT_SHADER, "garish-fs.glsl")
  val vsTrafo = Shader(gl, GL.VERTEX_SHADER, "trafo-vs.glsl")
  val garishProgram = Program(gl, vsTrafo, fsGarish)
  val modelMatrix = Mat4()
  val timeAtFirstFrame = Date().getTime()
  var timeAtLastFrame =  timeAtFirstFrame
  val asteroidTexture = Texture2D(gl, "media/asteroid.png")

  fun resize(gl : WebGL2RenderingContext, canvas : HTMLCanvasElement) {
    gl.viewport(0, 0, canvas.width, canvas.height)//#viewport# tell the rasterizer which part of the canvas to draw to ˙HUN˙ a raszterizáló ide rajzoljon
  }

  @Suppress("UNUSED_PARAMETER")
  fun update(gl : WebGL2RenderingContext, keysPressed : Set<String>) {
    gl.clearColor(0f, 0.5f, 1f, 1.0f)//## red, green, blue, alpha in [0, 1]
    gl.clearDepth(1.0f)//## will be useful in 3D ˙HUN˙ 3D-ben lesz hasznos
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)//#or# bitwise OR of flags

    gl.useProgram(solidProgram.glProgram)
    quadGeometry.draw()
    gl.useProgram(garishProgram.glProgram)

    val timeAtThisFrame = Date().getTime()
    val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
    timeAtLastFrame = timeAtThisFrame

    val velocity = Vec2(0.0f, 0.0f)

    if(keysPressed.contains("W")) {
      velocity.y = 1.0f
    }
    if(keysPressed.contains("S")) {
      velocity.y = -1.0f
    }
    if(keysPressed.contains("A")) {
      velocity.x = -1.0f
    }
    if(keysPressed.contains("D")) {
      velocity.x = 1.0f
    }

    if(keysPressed.contains("E")) {
      gl.uniform4f(
        gl.getUniformLocation(
          garishProgram.glProgram,
          "gameObject.color"),
        0.5f, 0.5f, 0.0f, 1.0f
      )
    }
    else {
        gl.uniform4f(
            gl.getUniformLocation(
            garishProgram.glProgram,
            "gameObject.color"),
            0.0f, 0.0f, 0.0f, 1.0f
        )
    }

    modelMatrix.translate(velocity.x * dt, velocity.y * dt);

    gl.uniformMatrix4fv(
      gl.getUniformLocation(
        garishProgram.glProgram,
        "gameObject.modelMatrix"),
      false,
      Float32Array(modelMatrix.storage)
    )

    gl.uniform1i(gl.getUniformLocation(
      garishProgram.glProgram, "material.colorTexture"), 0)
    gl.activeTexture(GL.TEXTURE0)
    gl.bindTexture(GL.TEXTURE_2D, asteroidTexture.glTexture)

    gl.enable(GL.BLEND)
    gl.blendFunc( GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA)w

    if(!keysPressed.contains("SPACE")) {
      triangleGeometry.draw()
    }
  }

  fun mouseMove(x: Float, y: Float){
    val ndc_x = x / gl.canvas.width * 2.0f - 1.0f
    val ndc_y = y / gl.canvas.height * -2.0f + 1.0f
    modelMatrix.translate(ndc_x, ndc_y)
  }
}
