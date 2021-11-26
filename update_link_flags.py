
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()

env.Append(
    LINKFLAGS=[
        "-mthumb",
        "-mfloat-abi=hard",
        "-mfpu=fpv4-sp-d16"
    ]
)