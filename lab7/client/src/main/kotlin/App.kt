import kotlinx.browser.document
import kotlinx.browser.window
import org.w3c.dom.HTMLDivElement
import org.w3c.dom.HTMLCanvasElement
import org.w3c.dom.events.*
import org.khronos.webgl.Uint32Array
import org.w3c.dom.Window

external class Socket {
  fun on(message : String, callback : dynamic)
  fun emit(message : String, data : dynamic)
}

external class Io {
  fun connect(url:String): Socket
}

external val io : Io


class App(val canvas : HTMLCanvasElement, val overlay : HTMLDivElement) {
  
  val gl = (canvas.getContext("webgl2", object{val antialias = false; val alpha = false}) ?: throw Error("Browser does not support WebGL2")) as WebGL2RenderingContext
  init {
    gl.getExtension("EXT_color_buffer_float") ?: throw Error("Browser does not support extension EXT_color_buffer_float")
  }
  val vars = HashMap<String, String>()
  var parts = Regex("[?&]+([^=&]+)=([^&]*)").findAll(window.location.href)
  init {
    parts.forEach { 
      vars[it.groupValues[1]] = it.groupValues[2]
      console.log(it.groupValues[1], it.groupValues[2])
    }
  }

  val keysPressed = HashSet<String>()

  val socket = io.connect("ws://localhost:3001")
  init {
    socket.on("welcome") { playerId : Int ->
      console.log("welcome in")
      val playerName = vars["name"] ?: "Player ${playerId}"
      val scene = Scene(gl, playerId, playerName)
      socket.emit("introduction", playerName)

      socket.on("syncState") { serializedData : String ->
        scene.applyServerState(serializedData)
      }

      document.onkeydown =  {
          event : KeyboardEvent ->
        keysPressed.add( keyNames[event.keyCode] )
        scene.onKeyDown( keyNames[event.keyCode] )
        socket.emit("keyDown", keyNames[event.keyCode] )
      }
      document.onkeyup = {
          event : KeyboardEvent ->
        keysPressed.remove( keyNames[event.keyCode] )
        scene.onKeyUp( keyNames[event.keyCode] )
        socket.emit("keyUp", keyNames[event.keyCode] )
      }
      val resize = {
        canvas.width = canvas.clientWidth
        canvas.height = canvas.clientHeight
        scene.resize(gl, canvas)
      }
      resize()
      window.addEventListener("resize", { resize() })
      fun update(){
        scene.update(gl, overlay, keysPressed)
        window.requestAnimationFrame{update()}
      }
      window.requestAnimationFrame{update()}

    }
  }


}

external object crypto {
  fun getRandomValues(array : Uint32Array)
}

fun main() {
  val canvas = document.getElementById("canvas") as HTMLCanvasElement
  val overlay = document.getElementById("overlay") as HTMLDivElement
  overlay.innerHTML = """<font color="red">WebGL</font>"""

  try{
    val app = App(canvas, overlay)
  } catch(e : Error) {
    console.error(e.message)
  }
}