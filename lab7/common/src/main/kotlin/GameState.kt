import kotlinx.serialization.Serializable
import vision.gears.webglmath.Vec1
import vision.gears.webglmath.Vec2
import kotlin.math.cos
import kotlin.math.exp
import kotlin.math.sin

@Serializable
class NetObject(
    val playerId: Int,
    val meshName: String,
    var label: String? = null
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

    var cooldown = 1f

    // LABTODO: cooldown
    // iMSc TODO: removal timer
    var shouldBeRemoved = false
    var removalTimer = 0f

    fun update(dt: Float, t: Float): Boolean {
        val ahead = Vec2(cos(roll), sin(roll))
        val right = Vec2(ahead.y, -ahead.x)

        velocity += (
                ahead * (aheadThrust - backThrust) +
                        right * (rightThrust - leftThrust)) * dt

        position += velocity * dt

        angVelocity += -cwTorque * dt + ccwTorque * dt

        roll += angVelocity * dt

        velocity *= exp(-dt)
        angVelocity *= exp(-dt)

        cooldown -= dt


        //LABTODO: Euler for velocity
        //LABTODO: exponential energy loss

        // LABTODO: cooldown
        // iMSc TODO: adjust removal timer, remove when expired
        if (shouldBeRemoved) {
            removalTimer -= dt
            if (removalTimer <= 0f) return false
        }

        return true
    }
}

@Serializable
class GameState() {

    val perPlayerObjectCounts =
        HashMap<Int, Int>()
    var objects =
        HashMap<Int, NetObject>()

    fun addObject(playerId: Int, meshName: String, label: String? = null): NetObject {
        val netObj = NetObject(playerId, meshName, label)
        val perPlayerObjectId = perPlayerObjectCounts[playerId] ?: 0
        perPlayerObjectCounts[playerId] = perPlayerObjectId + 1
        val netObjectId = perPlayerObjectId * 256 + playerId
        objects[netObjectId] = netObj
        return netObj
    }

    fun dropPlayer(playerId: Int) {
        // remove objects with a certain player id
        objects = HashMap(objects.filterValues { netObj ->
            netObj.playerId != playerId
        })
    }

    fun update(dt: Float, t: Float) {
        //objects.forEach{ (_ : Int, netObj : NetObject) -> netObj.update(dt, t)}
        objects = HashMap(objects.filterValues { it.update(dt, t) })

        //LABTODO: collision
        class GridEntry(val x: Int, val y: Int, val id: Int) : Comparable<GridEntry> {
            override fun compareTo(other: GridEntry): Int {
                if (x - other.x != 0) return x - other.x
                if (y - other.y != 0) return y - other.y
                return id - other.id
            }
        }

        val iObject = objects.iterator()
        if (!iObject.hasNext()) {
            return
        }
        var entry = iObject.next()
        val linearGrid = Array<GridEntry>(objects.size * 4) { i ->
            val netObj = entry.value
            val index = entry.key
            if (i % 4 == 3 && iObject.hasNext()) {
                entry = iObject.next()
            }
            GridEntry(
                netObj.position.x.toInt() / 2 - i % 2,
                netObj.position.y.toInt() / 2 - (i % 4) / 2,
                index
            )
        }
        linearGrid.sort()
        val candidates = HashSet<Pair<Int, Int>>()
        var j = 0
        while (j < linearGrid.size) {
            var k = j + 1
            while (k < linearGrid.size &&
                linearGrid[k].x == linearGrid[j].x &&
                linearGrid[k].y == linearGrid[j].y
            ) {
                candidates.add(Pair(linearGrid[j].id, linearGrid[k].id))
                k++
            }
            j++
        }
        candidates.forEach {
            val i = objects[it.first] ?: return@forEach
            val j = objects[it.second] ?: return@forEach
            val diff = i.position - j.position
            if (diff.dot(diff) < i.collisionRadius + j.collisionRadius) {
                val normal = diff.normalize()
                val relativeVelocity = (i.velocity - j.velocity).dot(normal)

                i.velocity -= normal * relativeVelocity
                j.velocity += normal * relativeVelocity

                val correction = normal * (i.collisionRadius + j.collisionRadius - diff.length())
                i.position += correction
                j.position -= correction

                if (i.meshName != "boom" && j.meshName != "boom") {
                    addObject(-1, "boom").apply {
                        position.set(i.position)
                        collisionRadius = -1f
                        shouldBeRemoved = true
                        removalTimer = 36 / 60f
                    }
                }
            }
        }

        val avatars = objects.filterValues { it.meshName == "fighter" }
        objects.filterValues { it.meshName == "asteroid" }.forEach { asteroid ->
            avatars.forEach { avatar ->
                val distance = (asteroid.value.position - avatar.value.position).length()
                if(distance <= 4f && asteroid.value.cooldown <= 0f) {
                    val direction = (avatar.value.position - asteroid.value.position).normalize()
                    asteroid.value.velocity.set(direction * 10f)
                }
            }
        }
    }

    fun onKeyDown(playerId: Int, key: String) {
        val obj = objects[playerId] ?: return
        if (key == "W") {
            obj.aheadThrust = 5.0f
        }
        if (key == "S") {
            obj.backThrust = 5.0f
        }
        if (key == "A") {
            obj.leftThrust = 2.0f
        }
        if (key == "D") {
            obj.rightThrust = 2.0f
        }
        if (key == "Q") {
            obj.ccwTorque = 2.0f
        }
        if (key == "E") {
            obj.cwTorque = 2.0f
        }
        if (key == "SPACE") {
            objects[playerId]?.let { avatar ->
                if (avatar.cooldown < 0.0f) {
                    val ahead = Vec2(cos(avatar.roll), sin(avatar.roll))
                    addObject(playerId, "orb").apply {
                        position.set(avatar.position + ahead)
                        velocity.set(ahead * 35f)
                    }
                    avatar.cooldown = 1.0f

                }
            }
        }
        if (key == "M") {
            objects[playerId]?.let { avatar ->
                if(avatar.cooldown < 0.0f) {
                    val ahead = Vec2(cos(avatar.roll), sin(avatar.roll))
                    addObject(playerId, "asteroid").apply {
                        position.set(avatar.position + ahead * 2f)
                        cooldown = 5f
                    }
                    avatar.cooldown = 2.0f
                }
            }
        }

    }

    fun onKeyUp(playerId: Int, key: String) {
        val obj = objects[playerId] ?: return
        if (key == "W") {
            obj.aheadThrust = 0.0f
        }
        if (key == "S") {
            obj.backThrust = 0.0f
        }
        if (key == "A") {
            obj.leftThrust = 0.0f
        }
        if (key == "D") {
            obj.rightThrust = 0.0f
        }
        if (key == "Q") {
            obj.ccwTorque = 0.0f
        }
        if (key == "E") {
            obj.cwTorque = 0.0f
        }
    }

    fun applyServerState(serverState: GameState) {
        objects = serverState.objects
    }

}