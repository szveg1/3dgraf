package vision.gears.webglmath

import org.khronos.webgl.WebGLTexture

interface Texture {
  abstract val glTexture : WebGLTexture?
}