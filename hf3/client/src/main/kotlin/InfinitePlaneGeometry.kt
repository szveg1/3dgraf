import org.khronos.webgl.Float32Array
import org.khronos.webgl.Uint16Array
import vision.gears.webglmath.Geometry
import org.khronos.webgl.WebGLRenderingContext as GL

class InfinitePlaneGeometry(val gl: WebGL2RenderingContext) : Geometry() {

    val vertexBuffer = gl.createBuffer()

    init {
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexBuffer)
        gl.bufferData(
            GL.ARRAY_BUFFER,
            Float32Array(
                arrayOf<Float>(
                    0f, 0f, 0f, 1f,
                    1f, 0f, 0f, 0f,
                    0f, 1f, 0f, 0f,
                    -1f, 0f, 0f, 0f,
                    0f, -1f, 0f, 0f
                )
            ),
            GL.STATIC_DRAW
        )
    }

    val vertexNormalBuffer = gl.createBuffer()

    init {
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexNormalBuffer)
        gl.bufferData(
            GL.ARRAY_BUFFER,
            Float32Array(
                arrayOf<Float>(
                    0f, 1f, 0f, 0f,
                    0f, 1f, 0f, 0f,
                    0f, 1f, 0f, 0f,
                    0f, 1f, 0f, 0f
                )
            ),
            GL.STATIC_DRAW
        )
    }

    val vertexTexCoordBuffer = gl.createBuffer()

    init {
        gl.bindBuffer(GL.ARRAY_BUFFER, vertexTexCoordBuffer)
        gl.bufferData(
            GL.ARRAY_BUFFER,
            Float32Array(
                arrayOf<Float>(
                    0f, 0f, 0f, 1f,
                    1f, 0f, 0f, 0f,
                    0f, 1f, 0f, 0f,
                    -1f, 0f, 0f, 0f,
                    0f, -1f, 0f, 0f
                )
            ),
            GL.STATIC_DRAW
        )
    }

    val indexBuffer = gl.createBuffer()

    init {
        gl.bindBuffer(GL.ELEMENT_ARRAY_BUFFER, indexBuffer)
        gl.bufferData(
            GL.ELEMENT_ARRAY_BUFFER,
            Uint16Array(
                arrayOf<Short>(
                    0, 1, 2,
                    0, 2, 3,
                    0, 3, 4,
                    0, 4, 1
                )
            ),
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
            4, GL.FLOAT,
            false,
            0,
            0
        )

        gl.bindBuffer(GL.ARRAY_BUFFER, vertexNormalBuffer)
        gl.enableVertexAttribArray(1)
        gl.vertexAttribPointer(
            1,
            4, GL.FLOAT,
            false,
            0,
            0
        )

        gl.bindBuffer(GL.ARRAY_BUFFER, vertexTexCoordBuffer)
        gl.enableVertexAttribArray(2)
        gl.vertexAttribPointer(
            2,
            4, GL.FLOAT,
            false,
            0,
            0
        )

        gl.bindVertexArray(null)
    }

    override fun draw() {
        gl.bindVertexArray(inputLayout)
        gl.bindBuffer(GL.ELEMENT_ARRAY_BUFFER, indexBuffer)

        gl.drawElements(GL.TRIANGLES, 12, GL.UNSIGNED_SHORT, 0)
    }
}
