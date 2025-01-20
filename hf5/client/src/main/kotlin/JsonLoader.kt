import org.w3c.xhr.XMLHttpRequest
import org.w3c.dom.events.*
import kotlinx.serialization.*
import kotlinx.serialization.json.*
import vision.gears.webglmath.Geometry

@Serializable
data class JsonMesh(
	val vertices : Array<Float>,
	val normals : Array<Float>,
	val texturecoords : Array<Array<Float>>,
	val faces : Array<Array<Short>>)

@Serializable
data class JsonModel(
	val meshes : Array<JsonMesh>)

@Serializable
data class JsonBone(
  val name : String,
  val offsetmatrix : Array<Float>,
  val weights : Array<Array<Float>>)

@Serializable
data class JsonRiggedMesh(
  val vertices : Array<Float>,
  val normals : Array<Float>,
  val texturecoords : Array<Array<Float>>,
  val faces : Array<Array<Short>>,
  val bones : Array<JsonBone>)

@Serializable
data class JsonRiggedModel(
  val meshes : Array<JsonRiggedMesh>)

class JsonLoader(){
  fun load(gl : WebGL2RenderingContext,
      jsonModelFileUrl : String) : Array<Geometry> {
    val request = XMLHttpRequest()
    request.overrideMimeType("application/json")
    request.open("GET", jsonModelFileUrl, false)
    var geometries : Array<Geometry>? = null
    request.onloadend = { 
      val json = Json { ignoreUnknownKeys=true }
      val jsonModel = json.decodeFromString(
                       JsonModel.serializer(), request.responseText)
      geometries = Array<Geometry>(jsonModel.meshes.size) {
        i -> SubmeshGeometry(gl, jsonModel.meshes[i])
      }
      Unit
    }
    request.send()
    return geometries!!
  }
  fun loadRigged(gl : WebGL2RenderingContext,
      jsonModelFileUrl : String) : Array<RiggedGeometry> {
    val request = XMLHttpRequest()
    request.overrideMimeType("application/json")
    request.open("GET", jsonModelFileUrl, false)
    var geometries : Array<RiggedGeometry>? = null
    request.onloadend = { 
      val json = Json { ignoreUnknownKeys=true }
      val jsonModel = json.decodeFromString(JsonRiggedModel.serializer(), request.responseText)
      geometries = Array<RiggedGeometry>(jsonModel.meshes.size) {
        i -> RiggedGeometry(gl, jsonModel.meshes[i])
      }
      Unit
    }
    request.send()
    return geometries!!
  }  
}