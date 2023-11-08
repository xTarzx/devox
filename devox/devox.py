import os
from enum import Enum, auto
from pathlib import Path
import subprocess


class LogLevel(Enum):
    INFO = auto(),
    WARNING = auto(),
    ERROR = auto()


def level_str(level: LogLevel):
    match level:
        case LogLevel.INFO:
            return "INFO"
        case LogLevel.WARNING:
            return "WARNING"
        case LogLevel.ERROR:
            return "ERROR"

    assert False, "UNREACHABLE"


def log(level: LogLevel, msg: str):
    print(f"[{level_str(level)}] {msg}")


class Devox:
    def __init__(self, project_name: str, project_root: str):
        self.project_root = os.path.relpath(project_root)
        self.build_dir = os.path.join(self.project_root, "devox_build")

        self.project_name = project_name
        self.exec_path = os.path.join(self.build_dir, self.project_name)

        self.src_dir = self.project_root
        self.inc_dir = self.project_root

        self.compiler = "g++"

        self.srcs: list[str] = []
        self.libs: list[str] = []

    def add_src(self, *srcs):
        for filename in srcs:
            fp = os.path.join(self.src_dir, filename)
            if not os.path.exists(fp):
                log(LogLevel.ERROR, f"'{filename}' not found")
                exit(1)
            self.srcs.append(filename)

    def link_lib(self, *libs):
        for lib in libs:
            self.libs.append(lib)

    def __libs_str(self) -> str:
        return " ".join([f"-l{lib}" for lib in self.libs])

    def build(self):
        if not os.path.exists(self.build_dir):
            log(LogLevel.INFO, "creating build directory")
            os.mkdir(self.build_dir)

        onames = []
        for src in self.srcs:
            obj_path = os.path.join(self.build_dir, Path(src).stem)
            oname = f"{obj_path}.o"
            onames.append(oname)
            cmd = f"{self.compiler} -o {oname} -c {src}"

            log(LogLevel.INFO, f"compiling '{src}'")
            log(LogLevel.INFO, f"CMD: {cmd}")
            res = subprocess.call(cmd.split())
            if res != 0:
                log(LogLevel.ERROR, f"failed to compile {src}")
                exit(1)

        cmd = f"{self.compiler} -o {self.exec_path} {' '.join(onames)} {self.__libs_str()}"
        log(LogLevel.INFO, f"linking '{self.exec_path}'")
        log(LogLevel.INFO, f"CMD: {cmd}")
        res = subprocess.call(cmd.split())
        if res != 0:
            log(LogLevel.ERROR, f"failed to link '{self.exec_path}'")
            exit(1)

        log(LogLevel.INFO, f"done")
