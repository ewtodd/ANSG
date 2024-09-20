import ROOT
import os
import sys

def combine_root_files(output_file, input_files):
    # Create a TChain for each tree type
    hits_chain = ROOT.TChain("Hits")
    energy_chain = ROOT.TChain("Energy")
    
    # Add files to the chains
    for file_name in input_files:
        hits_chain.Add(file_name)
        energy_chain.Add(file_name)
    
    # Check if we have entries
    if hits_chain.GetEntries() == 0 or energy_chain.GetEntries() == 0:
        print(f"No entries found in one of the trees. Hits: {hits_chain.GetEntries()}, Energy: {energy_chain.GetEntries()}")
        return
    
    # Create the output file
    output = ROOT.TFile(output_file, 'RECREATE')

    # Merge the chains into the output file
    output.cd()
    hits_tree = hits_chain.CloneTree(-1, "fast")
    energy_tree = energy_chain.CloneTree(-1, "fast")
    
    # Write the trees to the output file
    hits_tree.Write()
    energy_tree.Write()

    # Close the output file
    output.Close()
    print(f"Combined {len(input_files)} ROOT files into {output_file}")

# List of input ROOT files (one for each thread)
input_files = [f'build/output0_t{i}.root' for i in range(16)]

# Output ROOT file
if (len(sys.argv) != 1):
    output_file = sys.argv[1] + '.root'
else: output_file = 'combined_output.root'

# Combine the files
combine_root_files(output_file, input_files)
