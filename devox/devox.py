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

        self.inc_dir = self.project_root

        self.compiler = "g++"

        self.srcs: list[str] = []
        self.libs: list[str] = []

    def set_inc_dir_rel(self, path: str):
        self.inc_dir = os.path.join(self.project_root, path)

    def add_src(self, *srcs):
        for filename in srcs:
            fp = os.path.join(self.project_root, filename)
            if not os.path.exists(fp):
                log(LogLevel.ERROR, f"'{fp}' not found")
                exit(1)
            self.srcs.append(fp)

    def __obj_name(self, path: str) -> str:
        return f"{Path(path).stem}.o"

    def link_lib(self, *libs):
        for lib in libs:
            self.libs.append(lib)

    def __libs_str(self) -> str:
        return " ".join([f"-l{lib}" for lib in self.libs])

    def __is_modified_after(self, path1: str, path2: str) -> bool:
        if not os.path.exists(path1) or not os.path.exists(path2):
            return True

        return os.path.getmtime(path1) > os.path.getmtime(path2)

    def __compile(self, all: bool):
        for src in self.srcs:
            oname = self.__obj_name(src)
            obj_path = os.path.join(self.build_dir, oname)

            if all or self.__is_modified_after(src, obj_path):
                cmd = f"{self.compiler} -I{self.inc_dir} -o {obj_path} -c {src}"

                log(LogLevel.INFO, f"compiling '{src}'")
                log(LogLevel.INFO, f"CMD: {cmd}")
                res = subprocess.call(cmd.split())
                if res != 0:
                    log(LogLevel.ERROR, f"failed to compile {src}")
                    exit(1)
            else:
                log(LogLevel.INFO, f"skipping '{src}'")

    def __link(self):
        onames = [os.path.join(self.build_dir, self.__obj_name(src))
                  for src in self.srcs]
        cmd = f"{self.compiler} -o {self.exec_path} {' '.join(onames)} {self.__libs_str()}"
        log(LogLevel.INFO, f"linking '{self.exec_path}'")
        log(LogLevel.INFO, f"CMD: {cmd}")
        res = subprocess.call(cmd.split())
        if res != 0:
            log(LogLevel.ERROR, f"failed to link '{self.exec_path}'")
            exit(1)

    def ensure_build_folder(self):
        if not os.path.exists(self.build_dir):
            log(LogLevel.INFO, "creating build directory")
            os.mkdir(self.build_dir)

    def build(self, all: bool = False):
        self.ensure_build_folder()
        self.__compile(all)
        self.__link()
        log(LogLevel.INFO, f"done")
