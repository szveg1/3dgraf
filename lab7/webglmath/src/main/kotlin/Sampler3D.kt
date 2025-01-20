package vision.gears.webglmath

import org.khronos.webgl.Int32Array
import org.khronos.webgl.get
import org.khronos.webgl.set
import org.khronos.webgl.WebGLRenderingContext as GL
import org.khronos.webgl.WebGLUniformLocation
import org.khronos.webgl.WebGLTexture
import kotlin.reflect.KProperty

class Sampler3D : UniformSampler {

  override val storage = Int32Array(1)
  override val glTextures = Array<WebGLTexture?>(1) {null}

  operator fun provideDelegate(
      provider: UniformProvider,
      property: KProperty<*>) : Sampler3D {
    provider.register(property.name, this)
    return this
  }

  operator fun getValue(provider: UniformProvider, property: KProperty<*>): Sampler3D {
    return this
  }
  
  operator fun setValue(provider: UniformProvider, property: KProperty<*>, value: Sampler3D) {
    set(value)
  }

  override fun commit(gl : GL, uniformLocation : WebGLUniformLocation, samplerIndex : Int){
    storage[0] = samplerIndex;
    if(glTextures[0] != null) {
      gl.uniform1iv(uniformLocation, storage)
      gl.activeTexture(GL.TEXTURE0 + samplerIndex)
      gl.bindTexture(GL.TEXTURE_3D, glTextures[0])
    } else {
      throw Error("No texture bound to Sampler3D uniform.")
    }
  }

  override val glType : Int
    get() = GL.SAMPLER_3D   
}