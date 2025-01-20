package vision.gears.webglmath

import org.khronos.webgl.WebGLRenderingContext as GL
import org.khronos.webgl.WebGLUniformLocation

val GL.Companion.UNSIGNED_INT_SAMPLER_2D : Int get() = 0x8DD2
val GL.Companion.INT_SAMPLER_2D : Int get() = 0x8DCA
val GL.Companion.SAMPLER_2D_SHADOW : Int get() = 0x8B62
val GL.Companion.UNSIGNED_INT_SAMPLER_CUBE : Int get() = 0x8DD4
val GL.Companion.INT_SAMPLER_CUBE : Int get() = 0x8DCC
val GL.Companion.SAMPLER_CUBE_SHADOW : Int get() = 0x8DC5
val GL.Companion.UNSIGNED_INT_SAMPLER_3D : Int get() = 0x8DD3
val GL.Companion.INT_SAMPLER_3D : Int get() = 0x8DCB
val GL.Companion.SAMPLER_3D : Int get() = 0x8B5F
val GL.Companion.UNSIGNED_INT_SAMPLER_2D_ARRAY : Int get() = 0x8DD7
val GL.Companion.INT_SAMPLER_2D_ARRAY : Int get() = 0x8DCF
val GL.Companion.SAMPLER_2D_ARRAY_SHADOW : Int get() = 0x8DC4
val GL.Companion.SAMPLER_2D_ARRAY : Int get() = 0x8DC1

val GL.Companion.TEXTURE_3D : Int get() = 0x806F

interface Uniform {
  val glType : Int

  fun set(other: Uniform) : Uniform
  fun set(vararg values : Float) : Uniform
  fun set(firstTexture : Texture, vararg moreTextures : Texture)
  fun getStorageSize() : Int

  fun commit(gl : GL, uniformLocation : WebGLUniformLocation, samplerIndex : Int = 0)
}

