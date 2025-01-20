import org.khronos.webgl.WebGLRenderingContext as GL
import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint8Array
import org.khronos.webgl.set
import org.khronos.webgl.get
import vision.gears.webglmath.Geometry
import vision.gears.webglmath.Mat4
import org.w3c.xhr.XMLHttpRequest
import org.w3c.dom.events.*
import kotlin.math.max
import kotlinx.serialization.*
import kotlinx.serialization.json.*
import kotlinx.serialization.encoding.*

@Serializable
data class JsonNode(
  val name : String,
  val transformation : Array<Float>,
  val children : Array<JsonNode> = emptyArray<JsonNode>(),
  var parent : JsonNode? = null
  )

@Serializable
data class JsonKey(
  val first : Float,
  val second : Array<Float>) {
    @kotlinx.serialization.ExperimentalSerializationApi
    @Serializer(forClass = JsonKey::class)
    internal companion object : KSerializer<JsonKey> {
        override fun deserialize(decoder: Decoder): JsonKey {
          val input = decoder as? JsonDecoder ?: throw SerializationException("Expected Json Input")
          val array = input.decodeJsonElement() as? JsonArray ?: throw SerializationException("Expected JsonArray")
          val sarray = array[1] as? JsonArray ?: throw SerializationException("Expected JsonArray")
          return JsonKey((sarray[0] as? JsonPrimitive)?.float ?: 0.0f, 
            Array<Float>(sarray.size) {
              (sarray[it] as? JsonPrimitive)?.float ?: 0.0f
            }
          )
        }
    }
}

@Serializable
data class JsonChannel(
  val name : String,
  val scalingkeys  : Array<JsonKey>,  
  val rotationkeys : Array<JsonKey>,
  val positionkeys : Array<JsonKey> )

@Serializable
data class JsonAnimation(
  val channels : Array<JsonChannel>,
  val duration : Float )

@Serializable
data class JsonAnimatedHierarchy(
  val rootnode : JsonNode,
  val animations : Array<JsonAnimation> )

class Animation(
  val gl : WebGL2RenderingContext,
  val jsonAnimatedHierarchyUrl : String,
  val geometry : RiggedGeometry) {

  val skeleton = Uint8Array(geometry.nBones*16)
  val nodeNamesToNodeIndices = HashMap<String, Byte>()
  val nodeNamesToNodes = HashMap<String, JsonNode>()
  val boneTransformationChainNodeIndices = Array(geometry.nBones) {
  	ArrayList<Byte>()
  }

  var nNodes : Byte = 0
  var nKeys : Int = 0
  lateinit var keys : DualQuaternionArray

	init {
    val request = XMLHttpRequest();
    request.overrideMimeType("application/json")
    request.open("GET", jsonAnimatedHierarchyUrl, false)
    request.onloadend = {
      val json = Json { ignoreUnknownKeys=true }
      val jsonAnimatedHierarchy = json.decodeFromString(JsonAnimatedHierarchy.serializer(), request.responseText)
      for(i in 0 until geometry.nBones*16) {
        skeleton[i] = 255.toByte()
      }
      processNode(jsonAnimatedHierarchy.rootnode)

      jsonAnimatedHierarchy.animations.forEach { animation ->
        animation.channels.forEach { channel ->
          nKeys = max(nKeys, channel.rotationkeys.size)
          nKeys = max(nKeys, channel.positionkeys.size)
        }
      }
      keys = DualQuaternionArray(nNodes * nKeys)

	    nodeNamesToNodeIndices.forEach { (nodeName, iNode) ->
	      val node = nodeNamesToNodes[nodeName] ?: throw Error("Unknown node.")
	      val dq = DualQuaternion()
	      dq.fromMatrix(Mat4(*node.transformation.toFloatArray()).transpose())
	      keys.subarray(
	        iNode * nKeys,
	        iNode * nKeys + nKeys).fill(dq)
	    }

 	    jsonAnimatedHierarchy.animations.forEach { animation ->
	    	animation.channels.forEach { channel ->
	        val iNode = nodeNamesToNodeIndices[channel.name] ?: throw Error("Node has no index.")
	        channel.rotationkeys.forEachIndexed { iKey, key ->
	        	val dq = keys[iNode * nKeys + iKey]
            dq.setRotationFromArray( key.second )
            dq.setTranslationFromArray( channel.positionkeys[
            	  if (iKey<channel.positionkeys.size) iKey else (channel.positionkeys.size-1)
                ].second )
          }
        }
      }
    }
    request.send()
  }
	fun processNode(node : JsonNode) {
	  nodeNamesToNodeIndices[node.name] = nNodes
	  nodeNamesToNodes[node.name] = node
	  nNodes++
	  val iBone = geometry.boneNames.indexOf(node.name)
	  if(iBone >= 0) {
	    var pNode :JsonNode? = node
	    while(pNode != null){
	      val pNodeIndex = nodeNamesToNodeIndices[pNode.name] ?: throw Error("Node has no index.")
	      skeleton[iBone * 16 +
	         boneTransformationChainNodeIndices[iBone].size] =
	            pNodeIndex
	      boneTransformationChainNodeIndices[iBone].add( pNodeIndex )
	      pNode = pNode.parent
	    }
	  }
	  node.children.forEach { 
      it.parent = node
	    processNode(it);
    }
	}
}

