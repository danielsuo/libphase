import os
import subprocess
import sys

import click
import clickutil

from ..constants import LIBPHASE_HOME

SIMPOINT_HOME = os.path.join(
    LIBPHASE_HOME, "build",
    "deps/simpoint/simpoint-git-prefix/src/simpoint-git/")


def simpoint(bbv_file, k, max_k, variable_length, output_dir, verbose, quiet,
             args):

    print(locals())

    if not os.path.isabs(output_dir):
        output_dir = os.path.join(os.path.abspath(os.path.dirname(bbv_file)),
                                  output_dir)
        print(output_dir)

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    simpoint_binary = os.path.join(SIMPOINT_HOME, "bin/simpoint")

    cmd = [
        simpoint_binary, "-loadFVFile", bbv_file, "-saveSimpoints",
        os.path.join(output_dir, "out.simpoints"), "-saveSimpointWeights",
        os.path.join(output_dir, "out.weights"), "-saveFinalCtrs",
        os.path.join(output_dir, "out.centres"), "-saveLabels",
        os.path.join(output_dir, "out.labels"), "-saveVectorsTxtFmt",
        os.path.join(output_dir, "out.vectors")
    ]
    if k is not None:
        cmd.extend(["-k", k])
    else:
        cmd.extend(["-k", "search"])
        cmd.extend(["-maxK", str(max_k)])

    if variable_length:
        cmd.extend(["-fixedLength", "off"])

    if args is not None:
        args = [arg.replace("--", "-") for arg in args]
        cmd.extend(args)

    if verbose:
        print("Command: ", " ".join(cmd))

    proc = subprocess.Popen(cmd,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    out = proc.stdout.read()
    err = proc.stderr.read()
    proc.wait()

    if not quiet:
        print(out.decode("ascii"))
        print(err.decode("ascii"))

    return out, err


@click.command(context_settings=dict(help_option_names=["-h", "--help"],
                                     ignore_unknown_options=True,
                                     allow_extra_args=True))
@click.option("-b",
              "--bbv-file",
              default=os.path.join(SIMPOINT_HOME, "input/sample.bb"),
              type=click.Path(exists=True),
              help="Input bbv file")
@click.option("-k", default=None, type=str, help="Known number of simpoints")
@click.option("-m",
              "--max-k",
              default=30,
              type=int,
              help="Compute up to max k using binary search")
@click.option("--variable-length",
              type=bool,
              is_flag=True,
              help="Use variable length")
@click.option("-o",
              "--output-dir",
              default="out",
              type=click.Path(),
              help="Output directory")
@click.option("-v", "--verbose", is_flag=True, help="Enables verbose mode")
@click.option("-q", "--quiet", is_flag=True, help="Suppress output")
@click.argument("args", nargs=-1, type=click.UNPROCESSED)
@clickutil.call(simpoint)
def _simpoint():
    """
    Run simpoint tool
    """


if __name__ == "__main__":
    _simpoint()
