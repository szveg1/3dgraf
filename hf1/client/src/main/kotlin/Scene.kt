import org.khronos.webgl.Float32Array
import org.w3c.dom.HTMLCanvasElement
import vision.gears.webglmath.Mat4
import kotlin.js.Date
import org.khronos.webgl.WebGLRenderingContext as GL //# GL# we need this for the constants declared ˙HUN˙ a constansok miatt kell

class Scene (
  val gl : WebGL2RenderingContext){

  val vs = Shader(gl, GL.VERTEX_SHADER, "vs.glsl")
  val fs = Shader(gl, GL.FRAGMENT_SHADER, "fs.glsl")
  val shaderProgram = Program(gl, vs, fs)
  val quadGeometry = TexturedQuadGeometry(gl)
  val modelMatrix = Mat4().scale(0.1f, 0.1f, 1.0f)
  val textureTransform = Mat4(0.125f, 0.0f, 0.0f, 0.0f,
                              0.0f, 0.125f, 0.0f, 0.0f,
                              0.0f, 0.0f, 1.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 1.0f).translate(0.0f, 0.5f)
  val timeAtFirstFrame = Date().getTime()
  val texture = Texture2D(gl, "media/platformer_sprites_base.png")
  var timeAtLastFrame = timeAtFirstFrame

  var running = false;
  var facing = "RIGHT"
  var velocity = 0.0f
  var shouldMove = false
  var timer = 0.0f
  var seq = 0

  fun resize(gl : WebGL2RenderingContext, canvas : HTMLCanvasElement) {
    gl.viewport(0, 0, canvas.width, canvas.height)//#viewport# tell the rasterizer which part of the canvas to draw to ˙HUN˙ a raszterizáló ide rajzoljon
  }
  @Suppress("UNUSED_PARAMETER")
  fun update(gl : WebGL2RenderingContext, keysPressed : Set<String>) {
    gl.clearColor(0.0f, 0.4f, 1.0f, 1.0f)//## red, green, blue, alpha in [0, 1]
    gl.clearDepth(1.0f)//## will be useful in 3D ˙HUN˙ 3D-ben lesz hasznos
    gl.clear(GL.COLOR_BUFFER_BIT or GL.DEPTH_BUFFER_BIT)//#or# bitwise OR of flags

    gl.useProgram(shaderProgram.glProgram)

    gl.uniform1i(gl.getUniformLocation(
        shaderProgram.glProgram, "material.colorTexture"),0)
    gl.activeTexture(GL.TEXTURE0)
    gl.bindTexture(GL.TEXTURE_2D, texture.glTexture)

    val timeAtThisFrame = Date().getTime()
    val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
    timeAtLastFrame = timeAtThisFrame


    if(keysPressed.contains("SHIFT")){
      if(!running){
        running = true
        textureTransform.translate(0.0f, 0.5f)
      }
    }
    else{
      if(running){
        running = false
        textureTransform.translate(0.0f, -0.5f)
      }
    }

    if(keysPressed.contains("A")){
      if(facing != "LEFT"){
        facing = "LEFT"
        textureTransform.scale(-1.0f, 1.0f)
      }
      shouldMove = true
      velocity = if(running) -0.5f else -0.25f
    }
    else if(keysPressed.contains("D")){
      if(facing != "RIGHT"){
        facing = "RIGHT"
        textureTransform.scale(-1.0f, 1.0f)
      }
      shouldMove = true
      velocity = if(running) 0.5f else 0.25f
    }
    else{
      if(seq == 0) {
        shouldMove = false
        velocity = 0.0f
      }
    }

    if (timer > 0.05f && (shouldMove)){
      textureTransform.translate(0.125f)
      timer = 0.0f
      seq = (seq + 1) % 8
    } else {
      timer += dt
    }

    modelMatrix.translate(velocity * dt)
    gl.uniformMatrix4fv(
      gl.getUniformLocation(
        shaderProgram.glProgram,
        "gameObject.modelMatrix"),
      false,
      Float32Array(modelMatrix.storage)
    )

    gl.uniformMatrix4fv(
      gl.getUniformLocation(
        shaderProgram.glProgram,
        "gameObject.textureTransform"),
      false,
      Float32Array(textureTransform.storage)
    )

    quadGeometry.draw()
  }


}
