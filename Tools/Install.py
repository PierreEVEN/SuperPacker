

def IsWindows():
	from sys import platform
	return platform == "win32"

def IsLinux():
	from sys import platform
	return platform == "linux" or platform == "linux2"


import multiprocessing, os, platform, subprocess, sys

if IsWindows():
	import vswhere

ENGINE_PATH = os.getcwd()

THIRD_PARTY_PATH = ENGINE_PATH + "/Sources/ThirdParty/"
INSTALL_DIR = ENGINE_PATH + "/Intermediates/Dependencies"

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def LogError(message):
	print(bcolors.FAIL + "[E] %s" % message + bcolors.ENDC)
	sys.stdout.flush()
	
def PauseAssert():
	if 0 == sys.platform.find("win"):
		pauseCmd = "pause"
	else:
		pauseCmd = "read"
	subprocess.call(pauseCmd, shell = True)
	sys.exit(1)

def LogInfo(message):
	print(bcolors.OKCYAN + "[I] %s" % message + bcolors.ENDC)
	sys.stdout.flush()
	
def LogSuccess(message):
	print(bcolors.OKGREEN + "[I] %s" % message + bcolors.ENDC)
	sys.stdout.flush()
	
def LogWarning(message):
	print(bcolors.WARNING + "[W] %s" % message + bcolors.ENDC)
	sys.stdout.flush()
	
def CheckError(result):
	if result.returncode:
		if IsWindows():
			if result.stdout:
				LogWarning(str(result.stdout).replace("\\r\\n", "\n"))
			if result.stderr:
				LogError(str(result.stderr).replace("\\r\\n", "\n"))
		else:
			if result.stdout:
				LogWarning(str(result.stdout).replace("\\n", "\n"))
			if result.stderr:
				LogError(str(result.stderr).replace("\\n", "\n"))
		LogError("failed with exit code " + str(result.returncode))
		PauseAssert()

def RunSubProcess(Command):
    if IsWindows():
        CheckError(subprocess.run(Command, capture_output=True))
    else:
        CheckError(subprocess.run(Command.split(), capture_output=True))


def BuildModule(ModuleName, BuildProj = "ALL_BUILD.vcxproj", CMakeOptions = "", NinjaLibPath = "Null"):	
	if IsWindows():
		LibPath = THIRD_PARTY_PATH + ModuleName
		BuildPath = LibPath + "/Build"
		VcxProjPath = BuildPath + "/" + BuildProj
		# Search for MSBuild path
		VsPath = vswhere.find("MSBuild\**\Bin\MSBuild.exe")
		
		# Build module
		LogInfo("building module : " + ModuleName)
		RunSubProcess("cmake -S " + LibPath + " -B " + BuildPath + " " + CMakeOptions)
		
		# Compile module
		LogInfo("compiling ... ")
		RunSubProcess(VsPath[0] + " " + VcxProjPath + " /t:build /p:Configuration=\"Release\" /p:Platform=\"x64\" /p:BuildInParallel=true /p:OutDir=" + INSTALL_DIR)
		LogSuccess("Success !")
	if IsLinux():
		LibPath = THIRD_PARTY_PATH + ModuleName
		BuildPath = LibPath + "/Build"

		# Build module
		LogInfo("building module : " + ModuleName)
		RunSubProcess("cmake -S " + LibPath + " -G Ninja " + " -B " + BuildPath + " " + CMakeOptions)
		
		# Compile module
		LogInfo("compiling ... ")
		RunSubProcess("ninja -C " + BuildPath)
		LogSuccess("Success !")
		
		# Move libraries
		LogInfo("Move libraries")
		RunSubProcess("mv " + BuildPath + "/" + NinjaLibPath + " " + INSTALL_DIR)
		LogSuccess("Success !")




#MAIN

LogInfo("updating git submodules ...")
RunSubProcess("git submodule update --init --recursive")

RunSubProcess("mkdir -p " + INSTALL_DIR)

# Glfw
BuildModule(
	"glfw",
	"src/glfw.vcxproj",
	"",
	"src/libglfw3.a")

# Gl3w
LogInfo("downloading gl3w code ...")
os.chdir(THIRD_PARTY_PATH + "/gl3w")
RunSubProcess("python gl3w_gen.py")
os.chdir(ENGINE_PATH)

LogSuccess("Install complete !")
