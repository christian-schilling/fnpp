
env = Environment()

import os
env['ENV']['TERM'] = os.environ['TERM']

env["CXX"] = os.environ.get("CXX","clang++")

env.Append(
    CPPPATH=[
        "./src",
    ],
    CCFLAGS=[
        '-Ofast',
        '-std=c++11',
        '-Wall',
        '-Wextra',
        '-Werror',
    ],
)

env.Program("runtests",source=Glob("tests/*.cpp"))
env.Command("test_results","runtests","./runtests -a")
