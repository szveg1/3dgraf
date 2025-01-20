import vision.gears.webglmath.UniformProvider
import vision.gears.webglmath.Vec2
import vision.gears.webglmath.Mat4
import kotlin.math.exp
import kotlin.math.PI
import kotlin.math.floor
import kotlin.math.abs
import org.w3c.dom.HTMLDivElement

open class GameObject(
    val mesh: UniformProvider,
    val position: Vec2 = Vec2.zeros.clone(),
    var roll: Float = 0.0f,
    val scale: Vec2 = Vec2.ones.clone()
) : UniformProvider("gameObject") {

    var label: String? = null
    val modelMatrix by Mat4()
    val texOffset by Vec2()

    val animFreq = 25.0f;

    init {
        addComponentsAndGatherUniforms(mesh)
    }

    fun update() {
        modelMatrix.set().scale(scale).rotate(roll).translate(position)
    }

    fun interpolate(dt: Float, targetPosition: Vec2, targetRoll: Float) {
        val weight = exp(-dt * 100.0f)
        position.set(position * weight + targetPosition * (1 - weight))
        roll = roll * weight + targetRoll * (1 - weight)
    }

    open fun move(
        dt: Float = 0.016666f,
        t: Float = 0.0f,
        keysPressed: Set<String> = emptySet<String>(),
        gameObjects: List<GameObject> = emptyList<GameObject>()
    ): Boolean {

        val material = mesh.components[0]
        if (material != null && material is UniformProvider) {
            val texScale = material["texScale"] ?: Vec2.ones.clone()
            if (texScale is Vec2) {
                val phase = floor(t * animFreq) % (1 / texScale.x / texScale.y)
                texOffset.set(phase % (1 / texScale.x), floor(phase / (1 / texScale.x)))
            }
        }
        return true;
    }

    fun drawHTML(viewProjMatrix: Mat4, width: Int, height: Int): String {
        label?.let { label ->
            var ndc = position.xy01 * viewProjMatrix
            ndc = ndc / ndc.w
            ndc = if (abs(ndc.x) > 0.9f) ndc * 0.9f / abs(ndc.x) else ndc
            ndc = if (abs(ndc.y) > 0.95f) ndc * 0.95f / abs(ndc.y) else ndc
            val x = (ndc.x + 1.0f) * 0.5f * width
            val y = (ndc.y - 1.0f) * -0.5f * height

            return """<div style="outline: 1px solid orange; color:orange; background-color:black; position:absolute; left:${x.toInt()}px; top:${y.toInt()}px; transform: translate(-50%, -50%);"><nobr>${label}</nobr></div>"""
        } ?: return ""
    }
}


