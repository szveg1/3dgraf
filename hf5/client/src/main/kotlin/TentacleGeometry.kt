import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint16Array
import vision.gears.webglmath.Geometry
import kotlin.math.PI
import kotlin.math.cos
import kotlin.math.sin
import org.khronos.webgl.WebGLRenderingContext as GL

class TentacleGeometry(
    val gl: WebGL2RenderingContext,
    height: Float,
    radius: Float,
    segments: Int,
) : Geometry() {

    val unitCircleVertices = ArrayList<Float>()

    init {
        val segmentStep = 2 * PI.toFloat() / segments
        for (i in 0..segments) {
            val angle = i * segmentStep
            unitCircleVertices.addAll(listOf(cos(angle), 0f, sin(angle)))
        }
    }

    val vertices = ArrayList<Float>()
    val normals = ArrayList<Float>()
    val texCoords = ArrayList<Float>()
    val indices = ArrayList<Short>()

    init {
        for (i in 0 until 2) {
            val y = -height / 2.0f + i * height
            val v = 1.0f - i;

            for (j in 0..segments) {
                val k = j * 3
                val ux = unitCircleVertices[k]
                val uy = unitCircleVertices[k + 1]
                val uz = unitCircleVertices[k + 2]
                vertices.addAll(listOf(ux * radius, y, uz * radius))
                normals.addAll(listOf(ux, uy, uz))
                texCoords.addAll(listOf(j.toFloat() / segments, v))
            }
        }

        val baseCenterIndex = vertices.size / 3
        val topCenterIndex = baseCenterIndex + segments + 1

        for (i in 0 until 2) {
            val y = -height / 2.0f + i * height
            val ny = -1.0f + i.toFloat() * 2.0f
            vertices.addAll(listOf(0f, y, 0f))
            normals.addAll(listOf(0f, ny, 0f))
            texCoords.addAll(listOf(0.5f, 0.5f))

            for (j in 0 until segments) {
                val k = j * 3
                val ux = unitCircleVertices[k]
                val uz = unitCircleVertices[k + 2]
                vertices.addAll(listOf(ux * radius, y, uz * radius))
                normals.addAll(listOf(0f, ny, 0f))
                texCoords.addAll(
                    listOf(
                        -ux * 0.5f + 0.5f,
                        -uz * 0.5f + 0.5f
                    )
                )
            }
        }

        for (i in 0 until segments) {
            val k1 = i
            val k2 = segments + 1 + i
            indices.addAll(listOf(
                k1,
                k1 + 1,
                k2,
                k2,
                k1 + 1,
                k2 + 1
            ).map { it.toShort() })
        }

        for (i in 0 until segments) {
            val k = baseCenterIndex + 1 + i
            if (i < segments - 1) {
                indices.addAll(
                    listOf(
                        baseCenterIndex,
                        k + 1,
                        k
                    ).map { it.toShort() }
                )
            } else {
                indices.addAll(
                    listOf(
                        baseCenterIndex,
                        baseCenterIndex + 1,
                        k
                    ).map { it.toShort() }
                )
            }
        }

        for (i in 0 until segments) {
            val k = topCenterIndex + 1 + i
            if (i < segments - 1) {
                indices.addAll(
                    listOf(
                        topCenterIndex,
                        k,
                        k + 1
                    ).map { it.toShort() }
                )
            } else {
                indices.addAll(
                    listOf(
                        topCenterIndex,
                        k,
                        topCenterIndex + 1
                    ).map { it.toShort() }
                )
            }
        }
    }

    val vertexBuffer = gl.createBuffer()
    val vertexData = Float32Array(vertices.toTypedArray())

    init {
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexBuffer)
        gl.bufferData(
            GL.ARRAY_BUFFER,
            vertexData,
            GL.STATIC_DRAW
        )
    }


    val vertexNormalBuffer = gl.createBuffer()
    val normalData = Float32Array(normals.toTypedArray())

    init {
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexNormalBuffer)
        gl.bufferData(
            GL.ARRAY_BUFFER,
            normalData,
            GL.STATIC_DRAW
        )
    }


    val vertexTexCoordBuffer = gl.createBuffer()
    val texCoordData = Float32Array(texCoords.toTypedArray())

    init {
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexTexCoordBuffer)
        gl.bufferData(
            GL.ARRAY_BUFFER,
            texCoordData,
            GL.STATIC_DRAW
        )
    }


    val indexBuffer = gl.createBuffer()
    val indexData = Uint16Array(indices.toTypedArray())

    init {
        gl.bindBuffer(GL.ELEMENT_ARRAY_BUFFER, indexBuffer)
        gl.bufferData(
            GL.ELEMENT_ARRAY_BUFFER,
            indexData,
            GL.STATIC_DRAW
        )
    }


    val inputLayout = gl.createVertexArray()

    init {
        gl.bindVertexArray(inputLayout)

        gl.bindBuffer(GL.ARRAY_BUFFER, vertexBuffer)
        gl.enableVertexAttribArray(0)
        gl.vertexAttribPointer(
            0,
            3, GL.FLOAT, //< three pieces of float
            false, //< do not normalize (make unit length)
            0, //< tightly packed
            0 //< data starts at array start
        )
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexNormalBuffer)
        gl.enableVertexAttribArray(1)
        gl.vertexAttribPointer(
            1,
            3, GL.FLOAT, //< three pieces of float
            false, //< do not normalize (make unit length)
            0, //< tightly packed
            0 //< data starts at array start
        )
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexTexCoordBuffer)
        gl.enableVertexAttribArray(2)
        gl.vertexAttribPointer(
            2,
            2, GL.FLOAT, //< two pieces of float
            false, //< do not normalize (make unit length)
            0, //< tightly packed
            0 //< data starts at array start
        )

        gl.bindVertexArray(null)
    }

    override fun draw() {
        gl.bindVertexArray(inputLayout)
        gl.bindBuffer(GL.ELEMENT_ARRAY_BUFFER, indexBuffer)

        gl.drawElements(GL.TRIANGLES, indices.size, GL.UNSIGNED_SHORT, 0)
    }
}

