import org.jetbrains.kotlin.gradle.targets.js.yarn.YarnLockMismatchReport
import org.jetbrains.kotlin.gradle.targets.js.yarn.YarnRootExtension

rootProject.plugins.withType(org.jetbrains.kotlin.gradle.targets.js.yarn.YarnPlugin::class.java) {
    rootProject.the<YarnRootExtension>().yarnLockMismatchReport =
        YarnLockMismatchReport.WARNING // NONE | FAIL
    rootProject.the<YarnRootExtension>().reportNewYarnLock = false // true
    rootProject.the<YarnRootExtension>().yarnLockAutoReplace = false // true
}

plugins {//#plugins# apply plugins; already loaded in master project
  kotlin("multiplatform")
  kotlin("plugin.serialization")  
}

val buildDir = File(project.parent?.projectDir ?: project.projectDir, "build")
if(!buildDir.exists())
  buildDir.mkdir()//$ create build folder
project.layout.buildDirectory.fileValue( File(buildDir, "server"))
val webDir = File(buildDir, "web")
if(!webDir.exists())
  webDir.mkdir()//$ create build/web folder

kotlin {
  sourceSets {
    val commonMain by getting {
      kotlin.srcDir("src/main/kotlin")
      kotlin.srcDir("../webglmath/src/main/kotlin")
      kotlin.srcDir("../common/src/main/kotlin")
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-json-js:1.6.2")  
		implementation(kotlin("stdlib-js"))
		implementation(npm("express", "4.17.1"))
		implementation(npm("socket.io", "4.3.1"))
      }      
    }    
  }   
  js(IR) {
    binaries.executable()    
    nodejs {
      runTask {
        workingDir = project.parent?.projectDir ?: project.projectDir
        setArgs(listOf("--inspect"))
      }
    }
  }
}
