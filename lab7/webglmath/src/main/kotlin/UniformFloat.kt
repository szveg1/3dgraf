package vision.gears.webglmath
import org.khronos.webgl.Float32Array
import org.khronos.webgl.get
import org.khronos.webgl.set

interface UniformFloat : Uniform {
  abstract val storage : Float32Array

  override fun getStorageSize() : Int{
    return storage.length
  }

  override fun set(other: Uniform) : Uniform {
    if(other is UniformFloat){
      for(i : Int in 0 until storage.length) {
          storage[i] = if(i < other.storage.length) other.storage[i] else 0.0f
      }
    } else {
      throw Error("Cannot set a uniform of floats from a non-float uniform.")
    }
    return this
  }

  override fun set(firstTexture : Texture, vararg moreTextures : Texture){
    throw Error("Cannot set a texture to a non-sampler uniform.")
  }

}
