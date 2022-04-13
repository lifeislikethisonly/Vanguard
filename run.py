import argparse as ap
from collections import defaultdict
import os
import re
import shutil
import subprocess
from tabulate import tabulate

# NOTE: Update these with correct local paths!
SOLANG=os.path.join("/", "Users", "bmariano", "veridise", "solang", "target", "release", "solang")
PREPROCESSOR=os.path.join("deps", "SolidityPreprocessor", "src", "preprocess.js")
OPT_PATH = os.path.join("/", "opt", "homebrew", "Cellar", "llvm", "13.0.1_1", "bin", "opt")

DETECTORS = ["reentrancy", "suicide", "uninitialized_state",
             "msg_value_loop", "delegatecall_loop", "tainted_send", "flashloan"]

OUT_DIR="processed_examples"

IR_SOLANG_CMD="{} --target ewasm --emit llvm-ir -o {{}} {{}}".format(SOLANG)
BC_SOLANG_CMD="{} --target ewasm --emit llvm-bc -o {{}} {{}}".format(SOLANG)
PREPROCESSOR_CMD="node {} {{}}".format(PREPROCESSOR)
OPT_CMD = "{} --load=cmake-build-debug/lib/libVanguard.dylib -enable-new-pm=0 --summary={{}} --{{}} {{}} -o /dev/null".format(OPT_PATH)

def get_per_func_info(s: str):
    patt = re.compile("(Function '([^']*)' [^\n]*)")
    matches = patt.findall(s)

    return matches

def preprocess_file(args):
    file_name = os.path.basename(args.src_path)

    # Generate commandline command for Solang
    solang_cmd = BC_SOLANG_CMD.format(OUT_DIR, args.src_path)

    # Generate preprocessor command
    preprocessor_cmd = PREPROCESSOR_CMD.format(args.src_path,OUT_DIR)

    # Run solang
    print("Running Solang...", end="")
    subprocess.check_output(solang_cmd.split(), stderr=subprocess.DEVNULL)
    print("Completed Solang, compiled files in folder at {}".format(OUT_DIR))
    if args.create_ir:
        print("Running Solang to create IR (becaue --create_ir used!)...", end="")
        solang_ir_cmd = IR_SOLANG_CMD.format(OUT_DIR, args.src_path)
        subprocess.check_output(solang_ir_cmd.split(), stderr=subprocess.DEVNULL)
        print("Completed Solang, IR files in folder at {}".format(OUT_DIR))

    # Run preprocessor and save result to json
    summary_name = file_name.replace(".sol", "_summary.json")
    summary_path = os.path.join(OUT_DIR, summary_name)
    with open(summary_path, "wb") as f:
        print("Running Solidity preprocessor...", end="")
        summary = subprocess.check_output(preprocessor_cmd.split(), stderr=subprocess.DEVNULL)
        f.write(summary)
        print("Completed Solidity preprocessor, summary at {}.".format(summary_path))

    return summary_path

def run_detector(detector, summary_path):
    # Run detector!
    results = {}
    for bc_file in os.listdir(OUT_DIR):
        if bc_file.endswith("_deploy.bc"):
            # Only process bc files
            class_name = bc_file.replace("_deploy.bc", "")
            bc_path = os.path.join(OUT_DIR, bc_file)
            opt_cmd = OPT_CMD.format(summary_path, detector, bc_path)
            # print(opt_cmd)
            print("Running Vanguard on class {}...".format(class_name), end="")
            tout = bytes.decode(subprocess.check_output(opt_cmd.split(), stderr=subprocess.DEVNULL))
            print("Completed Vanguard.")
            results[class_name] = tout

    return results

def print_results(results):
    out = [["Class", "Detector", "Function Flagged by Detector", "Detailed Output"]]
    for cls, ress in results.items():
        for info in ress:
            detector, res = info
            parsed_res = get_per_func_info(res)
            for note, func in parsed_res:
                simple_func = func.split("::")[-1].split("__")[0]
                row = [cls, detector, simple_func, note]
                out.append(row)

    print(tabulate(out, tablefmt="grid"))

def run(args):
    if os.path.exists(OUT_DIR):
        # Remove existing processed files
        shutil.rmtree(OUT_DIR)
    # Make output directory
    os.makedirs(OUT_DIR)

    assert args.src_path != None, "No source path given (with --src_path arg)!"
    assert os.path.exists(args.src_path), "Source path {} does not exist!".format(args.src_path)
    assert args.detector != None, "No detector given (with --detector arg)!"
    assert args.detector in DETECTORS or args.detector=='all', "{} is not a recognized detector (check DETECTORS at top of run.py)!".format(args.detector)

    if args.detector == 'all':
        detectors = DETECTORS
    else:
        detectors = [args.detector]

    print("----Preprocessing {}----".format(args.src_path))
    summary_path = preprocess_file(args)

    results = defaultdict(list)
    for detector in detectors:
        print("----Running {} with {} detector----".format(args.src_path, detector))
        detector_results = run_detector(detector, summary_path)
        for cls, res in detector_results.items():
            results[cls].append((detector, res))

    print("----VANGUARD REPORT----")
    print_results(results)

if __name__=="__main__":
    parser = ap.ArgumentParser()
    parser.add_argument("--src_path", help="path to program to run (default is None)", type=str, default=None)
    parser.add_argument("--detector", help="choose benchmark to curate (default is None, 'all' will run all detectors)", type=str, default=None)
    parser.add_argument("--create_ir", help="enables output of ir in addition to bc files", action="store_true")
    args = parser.parse_args()
    run(args)
