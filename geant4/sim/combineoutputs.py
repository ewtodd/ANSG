import ROOT
import os

def combine_root_files(output_file, input_files):
    # Open the first file to initialize the output file structure
    first_file = ROOT.TFile(input_files[0], 'READ')
    if not first_file or first_file.IsZombie():
        print(f"Error: Unable to open the first input file {input_files[0]}")
        return
    
    output = ROOT.TFile(output_file, 'RECREATE')
    if not output or not output.IsWritable():
        print(f"Error: Unable to create or write to the output file {output_file}")
        return
    
    trees = {}
    histos = {}

    first_file.cd()
    for key in first_file.GetListOfKeys():
        obj = key.ReadObj()
        obj_name = obj.GetName()
        if obj.IsA().InheritsFrom('TTree'):
            # Clone an empty version of the TTree in the output file
            tree = obj.CloneTree(0)
            if not tree:
                print(f"Error: Failed to clone empty TTree {obj_name}")
                continue
            tree.SetDirectory(output)
            trees[obj_name] = tree
        elif obj.IsA().InheritsFrom('TH1'):
            h1 = obj.Clone()
            if h1:
                h1.SetDirectory(output)
                histos[obj_name] = h1
            else:
                print(f"Error: Failed to clone TH1 {obj_name}")
        else:
            continue
    first_file.Close()
    
    # Merge contents from all input files
    for name in input_files:
        file = ROOT.TFile(name, 'READ')
        if not file or file.IsZombie():
            print(f"Error: Unable to open input file {name}")
            continue
        
        file.cd()
        for key in file.GetListOfKeys():
            obj = key.ReadObj()
            obj_name = obj.GetName()
            output.cd()
            
            if obj.IsA().InheritsFrom('TTree'):
                tree = trees.get(obj_name)
                if not tree:
                    print(f"Error: TTree {obj_name} not found in the output file.")
                    continue
                temp_tree = obj.CloneTree(-1, 'fast')
                if not temp_tree:
                    print(f"Error: Failed to clone TTree {obj_name}")
                    continue
                tree.CopyEntries(temp_tree)
                temp_tree.SetDirectory(0)
                temp_tree.Delete()
            elif obj.IsA().InheritsFrom('TH1'):
                h1 = histos.get(obj_name)
                if h1:
                    h1.Add(obj)
                else:
                    print(f"Error: TH1 {obj_name} not found in the output file.")
        file.Close()

    # Write the combined contents to the output file
    output.Write()
    output.Close()

# List of input ROOT files (one for each thread)
input_files = [
    'build/output0_t0.root',
    'build/output0_t1.root',
    'build/output0_t2.root',
    'build/output0_t3.root',
    'build/output0_t4.root',
    'build/output0_t5.root',
    'build/output0_t6.root',
    'build/output0_t7.root',
    'build/output0_t8.root',
    'build/output0_t9.root',
    'build/output0_t10.root',
    'build/output0_t11.root',
    'build/output0_t12.root',
    'build/output0_t13.root',
    'build/output0_t14.root',
    'build/output0_t15.root'
    # Add more files as necessary
]

# Output ROOT file
output_file = 'combined_output.root'

# Combine the files
combine_root_files(output_file, input_files)

print(f"Combined {len(input_files)} ROOT files into {output_file}")

