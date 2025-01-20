import vision.gears.webglmath.*

class SpotLight(id : Int) : UniformProvider("spotlights[$id]") {
    val position by Vec3()
    val direction by Vec3()
    val powerDensity by Vec3()
}