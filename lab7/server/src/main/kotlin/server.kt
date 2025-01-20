import kotlin.js.Date
import GameState
import kotlinx.serialization.*
import kotlinx.serialization.json.*

external fun require(module:String):dynamic
external fun setInterval(callback: (args: Any) -> Unit, ms: Number, vararg args: Any): dynamic

fun main(args: Array<String>) {
  println("Hello JavaScript!")

  val express = require("express")
  val app = express()
  app.use(express.static("build/web"))

  val gameState = GameState()

  var server : dynamic = null
  server  = app.listen(3001) {
    console.log("Server launched.")
    if(server != null) {
      console.log("Server is not null.")
      val io = require("socket.io")(server)
      console.log("Socket open.")
      if(io != null) {
        console.log("Socket is not null.")
      }

      var lastClientId = 0
      io.on("connection") { client ->
        val clientId = lastClientId++
        console.log("User connected, id: ${clientId}")

        client.on("introduction") { name : String ->
          gameState.objects[clientId]?.label = name
          undefined
        }

        client.on("disconnect", {
          console.log("User left, id: ${clientId}")
          gameState.dropPlayer(clientId)
          undefined
        })

        client.on("keyDown") { key : String ->
          gameState.onKeyDown(clientId, key)
          undefined
        }

        client.on("keyUp") { key : String ->
          gameState.onKeyUp(clientId, key)
          undefined
        }

        gameState.addObject(clientId, "fighter", "Player ${clientId}")
        client.emit("welcome", clientId )
      }

      val timeAtFirstFrame = Date().getTime()
      var timeAtLastFrame =  timeAtFirstFrame
      val json = Json { ignoreUnknownKeys=true }
      setInterval( {
        val timeAtThisFrame = Date().getTime()
        val dt = (timeAtThisFrame - timeAtLastFrame).toFloat() / 1000.0f
        val t  = (timeAtThisFrame - timeAtFirstFrame).toFloat() / 1000.0f
        timeAtLastFrame = timeAtThisFrame
        gameState.update(dt, t)
        val serializedGameState = json.encodeToString(
          GameState.serializer(),
          gameState)

        io.emit("syncState", serializedGameState)
      } , 16)


    }
  }
  val port = server.address().port;
  console.log("Server listening at port ${port}")

}