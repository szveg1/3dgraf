import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint32Array
import vision.gears.webglmath.Vec1
import vision.gears.webglmath.Vec2
import vision.gears.webglmath.Mat4
import kotlin.math.exp
import kotlin.math.sin
import kotlin.math.cos
import kotlinx.serialization.*
import kotlinx.serialization.json.*

@Serializable
class NetObject (
  val playerId : Int,
  val meshName : String,
  var label : String? = null
  ) {

  val position = Vec2(128.0f, 128.0f + playerId.toFloat() * 2.2f)
  var roll = Vec1.makeRandom(-3.14f, +3.14f).x

  val velocity = Vec2.makeRandom(-0.1f, +0.1f)
  var angVelocity = Vec1.makeRandom(-0.314f, +0.314f).x

  var aheadThrust = 0.0f
  var backThrust = 0.0f  
  var rightThrust = 0.0f
  var leftThrust = 0.0f    
  var ccwTorque = 0.0f
  var cwTorque = 0.0f

  var collisionRadius = 1.0f

  // LABTODO: cooldown
  var cooldown = 0f
  // iMSc TODO: removal timer

  fun update(dt : Float, t : Float) : Boolean{
    val ahead = Vec2(cos(roll), sin(roll))
    val right = Vec2(ahead.y, -ahead.x)

    velocity += (
      ahead * (aheadThrust - backThrust) +
      right * (rightThrust - leftThrust) ) * dt

    position += velocity * dt

    //LABTODO: Euler for velocity
    //LABTODO: exponential energy loss
    velocity *= exp(-dt * 0.5f)

    roll += (ccwTorque - cwTorque) * dt

    // LABTODO: cooldown
    // iMSc TODO: adjust removal timer, remove when expired
    return true
  }
}

@Serializable
class GameState () {

  val perPlayerObjectCounts = 
    HashMap<Int, Int>()
  var objects = 
    HashMap<Int, NetObject>()

  fun addObject(playerId : Int, meshName : String, label : String? = null) : NetObject {
    val netObj = NetObject(playerId, meshName, label)
    val perPlayerObjectId = perPlayerObjectCounts[playerId]?:0
    perPlayerObjectCounts[playerId] = perPlayerObjectId + 1
    val netObjectId = perPlayerObjectId * 256 + playerId
    objects[netObjectId] = netObj
    return netObj
  }

  fun dropPlayer(playerId : Int){
    // remove objects with a certain player id
    objects = HashMap(objects.filterValues {
      netObj -> netObj.playerId != playerId
    })
  }

  fun update(dt : Float, t : Float) {
    //objects.forEach{ (_ : Int, netObj : NetObject) -> netObj.update(dt, t)}
    objects = HashMap(objects.filterValues{ it.update(dt, t) })

    //LABTODO: collision
  }

  fun onKeyDown(playerId : Int, key : String) {
    val obj = objects[playerId] ?: return
    if(key == "W"){
      obj.aheadThrust = 5.0f
    }
    if(key == "S"){
      obj.backThrust = 5.0f
    }
    if(key == "A"){
      obj.leftThrust = 2.0f
    }
    if(key == "D"){
      obj.rightThrust = 2.0f
    }    
    if(key == "Q"){
      obj.ccwTorque = 2.0f
    }
    if(key == "E"){
      obj.cwTorque = 2.0f
    }
    if(key == "SPACE") {
      objects[playerId]?.let{ avatar ->
        if(avatar.cooldown < 0.0f) {
          val ahead = Vec2(cos(avatar.roll), sin(avatar.roll))
          addObject(playerId, "orb").apply{
            position.set(avatar.position)
            velocity.set(ahead * 25f)
          }
          avatar.cooldown = 1.0f
          //cooldown-t a NetObjectbe vegyük fel, és folyamatosan csökkentsük
        }
        avatar.cooldown -= 0.1f
      }
    }

  }

  fun onKeyUp(playerId : Int, key : String) {
    val obj = objects[playerId] ?: return
    if(key == "W"){
      obj.aheadThrust = 0.0f
    }
    if(key == "S"){
      obj.backThrust = 0.0f
    }
    if(key == "A"){
      obj.leftThrust = 0.0f
    }
    if(key == "D"){
      obj.rightThrust = 0.0f
    }    
    if(key == "Q"){
      obj.ccwTorque = 0.0f
    }
    if(key == "E"){
      obj.cwTorque = 0.0f
    }
  }  

  fun applyServerState(serverState : GameState){
    objects = serverState.objects
  }

}