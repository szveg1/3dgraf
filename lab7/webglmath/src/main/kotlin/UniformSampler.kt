package vision.gears.webglmath
import org.khronos.webgl.Int32Array
import org.khronos.webgl.get
import org.khronos.webgl.set
import org.khronos.webgl.WebGLTexture

interface UniformSampler : Uniform {
  abstract val storage : Int32Array
  abstract val glTextures : Array<WebGLTexture?>

  override fun getStorageSize() : Int{
    return storage.length
  }

  override fun set(other: Uniform) : Uniform {
    if(other is UniformSampler){
      for(i : Int in 0 until storage.length) {
          storage[i] = if(i < other.storage.length) other.storage[i] else 0
          glTextures[i] = other.glTextures.getOrNull(i)
      }
    } else {
      throw Error("Cannot set a uniform of textures from a non-texture uniform.")
    }
    return this
  }

  override fun set(firstTexture : Texture, vararg moreTextures : Texture){
    glTextures[0] = firstTexture.glTexture
    for(i : Int in 0 until storage.length) {
      glTextures[i+1] = moreTextures.getOrNull(i)?.glTexture
    }
  }

  override fun set(vararg values : Float) : UniformFloat {
    throw Error("Cannot set float values to a texture uniform.")
  }
}

