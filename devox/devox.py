import os
from enum import Enum, auto
from pathlib import Path
import subprocess
from threading import Thread
from queue import Queue

from time import time

THREAD_COUNT = 4


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

        self.inc_dirs: list[str] = []
        self.link_dirs: list[str] = []

        self.objs: list[str] = []

        self.compiler = "g++"

        self.srcs: list[str] = []
        self.libs: list[str] = []

    def add_inc_dir_rel(self, *paths):
        for path in paths:
            fp = os.path.join(self.project_root, path)
            if not os.path.exists(fp):
                log(LogLevel.ERROR, f"'{fp}' not found")
                exit(1)
            self.inc_dirs.append(fp)

    def add_link_dir_rel(self, *paths):
        for path in paths:
            fp = os.path.join(self.project_root, path)
            if not os.path.exists(fp):
                log(LogLevel.ERROR, f"'{fp}' not found")
                exit(1)
            self.link_dirs.append(fp)

    def add_obj_rel(self, *paths):
        for path in paths:
            fp = os.path.join(self.project_root, path)
            if not os.path.exists(fp):
                log(LogLevel.ERROR, f"'{fp}' not found")
                exit(1)
            self.objs.append(fp)

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

    def __inc_dirs_str(self) -> str:
        return " ".join([f"-I{path}" for path in self.inc_dirs])

    def __link_dirs_str(self) -> str:
        return " ".join([f"-L{path}" for path in self.link_dirs])

    @staticmethod
    def __is_modified_after(path1: str, path2: str) -> bool:
        if not os.path.exists(path1) or not os.path.exists(path2):
            return True

        return os.path.getmtime(path1) > os.path.getmtime(path2)

    @staticmethod
    def __compile_worker(q: Queue, failed):
        while not q.empty():
            src, cmd = q.get()
            log(LogLevel.INFO, f"compiling '{src}'")
            log(LogLevel.INFO, f"CMD: {cmd}")
            res = subprocess.call(cmd.split(), stdout=subprocess.DEVNULL)
            if res != 0:
                failed.append(src)

    def __compile_threaded(self, all: bool):
        q = Queue()
        failed = []
        for src in self.srcs:
            oname = self.__obj_name(src)
            obj_path = os.path.join(self.build_dir, oname)
            if all or self.__is_modified_after(src, obj_path):
                cmd = f"{self.compiler} {self.__inc_dirs_str()} -o {obj_path} -c {src}"
                q.put((src, cmd))
            else:
                log(LogLevel.INFO, f"skipping '{src}'")
        threads: list[Thread] = []

        for _ in range(THREAD_COUNT):
            threads.append(
                Thread(target=Devox.__compile_worker, args=(q, failed)))

        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()

        if len(failed):
            for src in failed:
                # TODO: SHOW STDERR
                log(LogLevel.ERROR, f"failed to compile {src}")
            exit(1)

    def __compile(self, all: bool):
        for src in self.srcs:
            oname = self.__obj_name(src)
            obj_path = os.path.join(self.build_dir, oname)

            if all or self.__is_modified_after(src, obj_path):
                cmd = f"{self.compiler} {self.__inc_dirs_str()} -o {obj_path} -c {src}"

                log(LogLevel.INFO, f"compiling '{src}'")
                log(LogLevel.INFO, f"CMD: {cmd}")
                res = subprocess.call(cmd.split(), stdout=subprocess.DEVNULL)
                if res != 0:
                    log(LogLevel.ERROR, f"failed to compile {src}")
                    exit(1)
            else:
                log(LogLevel.INFO, f"skipping '{src}'")

    def __link(self):
        onames = [os.path.join(self.build_dir, self.__obj_name(src))
                  for src in self.srcs]
        cmd = f"{self.compiler} {self.__link_dirs_str()} -o {self.exec_path} {' '.join(onames)} {' '.join(self.objs)} {self.__libs_str()}"
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

    def build(self, all: bool = False, thread: bool = False):
        self.ensure_build_folder()
        start = time()
        if thread:
            self.__compile_threaded(all)
        else:
            self.__compile(all)
        self.__link()
        end = time()
        log(LogLevel.INFO, f"done in {end-start:.2f}s")
