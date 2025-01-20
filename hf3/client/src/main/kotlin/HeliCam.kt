import vision.gears.webglmath.Vec3

class HeliCam(
    var position: Vec3 = Vec3(),
    val velocity: Vec3 = Vec3(),
    val damping: Float = 0.9f,
    val springConstant: Float = 0.5f,
    val torque: Vec3 = Vec3(),
    val angularVelocity: Vec3 = Vec3(),
    val angularDamping: Float = 0.9f,
    val angularSpringConstant: Float = 0.5f
) {
    fun update(dt: Float, targetPosition: Vec3, targetDirection: Vec3) {
        val displacement = targetPosition - position
        val springForce = displacement * springConstant
        val dampingForce = velocity * -damping
        val force = springForce + dampingForce
        velocity += force * dt
        position.plusAssign(velocity * dt)

        val distance = (targetPosition - position).length()
        val minDistance = 30.0f
        if (distance < minDistance) {
            val direction = (position - targetPosition).normalize()
            position = targetPosition + direction * minDistance
        }

        val targetOrientation = targetDirection.normalize()
        val currentOrientation = torque.normalize()
        val angularDisplacement = targetOrientation.cross(currentOrientation)
        val angularSpringTorque = angularDisplacement * angularSpringConstant
        val angularDampingTorque = angularVelocity * -angularDamping
        val angularForce = angularSpringTorque + angularDampingTorque
        angularVelocity += angularForce * dt
        torque += angularVelocity * dt
    }
}