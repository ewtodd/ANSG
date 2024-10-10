import ROOT
import os
import sys

def combine_root_files(output_file, input_files):
    # Create a TChain for each tree type
    hits_chain = ROOT.TChain("Hits")
    energyCZT_chain = ROOT.TChain("EnergyCZT")
    energyHPGe_chain = ROOT.TChain("EnergyHPGe")
    energySiLi_chain = ROOT.TChain("EnergySiLi")
    
    # Add files to the chains
    for file_name in input_files:
        hits_chain.Add(file_name)
        energyCZT_chain.Add(file_name)
        energyHPGe_chain.Add(file_name)
        energySiLi_chain.Add(file_name)
    
        
    # Create the output file
    output = ROOT.TFile(output_file, 'RECREATE')

    # Merge the chains into the output file
    output.cd()
    hits_tree = hits_chain.CloneTree(-1, "fast")
    energyCZT_tree = energyCZT_chain.CloneTree(-1, "fast")
    energyHPGe_tree = energyHPGe_chain.CloneTree(-1, "fast")
    energySiLi_tree = energySiLi_chain.CloneTree(-1, "fast")

    
    # Write the trees to the output file
    hits_tree.Write()
    energyCZT_tree.Write()
    energyHPGe_tree.Write()
    energySiLi_tree.Write()

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
