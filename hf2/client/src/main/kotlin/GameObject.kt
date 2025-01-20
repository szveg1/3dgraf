import vision.gears.webglmath.UniformProvider
import vision.gears.webglmath.*


open class GameObject(vararg meshes : Mesh) : UniformProvider("gameObject") {

  val position = Vec3()
  var roll = 0.0f 
  val scale = Vec3(1.0f, 1.0f, 1.0f)
  /// Added for hit detection
  var radius = 0.0f

  val modelMatrix by Mat4()

  init {
    addComponentsAndGatherUniforms(*meshes)
  }

  fun update(){
    modelMatrix.set()
    modelMatrix.scale(scale).rotate(roll, Vec3(0.0f, 0.0f, 1.0f)).translate(position)
  }

  open class Motion(val gameObject : GameObject){
    open operator fun invoke(
      dt : Float,
      t : Float,
      keysPressed : Set<String>,
      interactors : ArrayList<GameObject>,
      spawn : ArrayList<GameObject>
      ) : Boolean {
      return true
    }
  }
  var move = Motion(this)

}