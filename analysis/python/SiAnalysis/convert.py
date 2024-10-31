import ROOT
import numpy as np
import time
import struct
import matplotlib.pyplot as plt

header_names = [
    'Record Length   ',    # 0
    'Board ID        ',    # 1
    'Pattern         ',    # 2
    'Channel         ',    # 3
    'Event Counter   ',    # 4
    'Timestamp       ',    # 5
    'DC offset       ',    # 6
    'Start Index Cell'     # 7
]

def PrintFirstEvent(filename):
    with open(filename, 'rb') as file:
        # Read first header
        header_length = file.read(4)

        event_size = struct.unpack('I', header_length)[0]
        sample_size = int(event_size/4-8)
        
        print(f"{header_names[0]}: {sample_size}")
        headers = struct.unpack('<7I', file.read(4 * 7))  # 'I' is the format character for an unsigned 32-bit integer

        for i, header in enumerate(headers):
            print(f"{header_names[i+1]}: {header}")
        
        # Now read the 1024 data points (32-bit float each)
        data_points = struct.unpack(f'{sample_size}f', file.read(sample_size * 4))  # 'f' is the format character for a 32-bit float

        # Print the data points
        for i, data_point in enumerate(data_points):
            print(f"{i}      {data_point}")

def ReadFullEvents(filename):
    stime = time.time()
    timestamps = []
    samples = []
    event_numbers =[]
    with open(filename, 'rb') as file:
        while True:
            # Read first header
            header_length = file.read(4)
            if len(header_length) < 4: break
        
            event_size = struct.unpack('I', header_length)[0]
            sample_size = int(event_size/4-8)
            sample = []
            
            headers = struct.unpack('<7I', file.read(4 * 7))  # 'I' is the format character for an unsigned 32-bit integer

            event_numbers.append(headers[3])
            timestamps.append(headers[4])
            
            # Now read the data points (32-bit float each)
            data_points = struct.unpack(f'{sample_size}f', file.read(sample_size * 4))  # 'f' is the format character for a 32-bit float
    
            sample.extend(data_points)
            samples.append(sample)

    etime = time.time()
    print(etime - stime)
    return event_numbers, timestamps, np.array(samples)

def SaveToROOT(filename, event_numbers, timestamps, samples):
    # Create a new ROOT file
    root_file = ROOT.TFile(filename, "RECREATE")

    # Create a TTree
    tree = ROOT.TTree("tree", "Tree with events")

    # Define data structure
    max_samples_length = len(samples[0])
    event_number = np.zeros(1, dtype=np.int32)
    timestamp = np.zeros(1, dtype=np.int32)
    sample_array = np.zeros(max_samples_length, dtype=np.float32)

    # Create branches
    tree.Branch("event_number", event_number, "event_number/I")
    tree.Branch("timestamp", timestamp, "timestamp/I")
    tree.Branch("samples", sample_array, f"samples[{max_samples_length}]/F")

    # Fill the tree
    for i in range(len(event_numbers)):
        event_number[0] = event_numbers[i]
        timestamp[0] = timestamps[i]
        sample_array[:] = samples[i]
        tree.Fill()

    # Write and close the file
    tree.Write()
    root_file.Close()
    print(f"Data saved to {filename}")

def PlotSamples(samples, indices):
    plt.figure(figsize=(12, 6))
    sample_times = 0.2*np.arange(0, 1024) # nano seconds, 5 GHz sampling rate
   
    for idx in indices:
        if idx < len(samples):
            plt.plot(sample_times, samples[idx], label=f"Sample {idx}")
        else:
            print(f"Index {idx} is out of range for the samples array.")

    plt.xlabel("Time (ns)")
    plt.ylabel("Amplitude")
    plt.title("Waveform Samples")
    #plt.legend()
    plt.show()

def PlotAverageSample(samples):
    plt.figure(figsize=(12, 6))
    sample_times = 0.2*np.arange(0, 1024) # nano seconds, 5 GHz sampling rate
    avg = np.mean(samples, axis=0)
    plt.plot(sample_times, avg)   
    plt.xlabel("Time (ns)")
    plt.ylabel("Amplitude")
    plt.title("Average Waveform")
    plt.show()



if __name__ == "__main__":
    input_filename = "TR_0_0.dat"  # Replace with your actual input filename
    output_filename = "output.root"

    #PrintFirstEvent(input_filename)
    event_numbers, timestamps, samples = ReadFullEvents(input_filename)
    #SaveToROOT(output_filename, event_numbers, timestamps, samples)


    indices_to_plot = np.arange(0, len(event_numbers)) # [0, 1]  # Indices of the samples you want to plot
    PlotSamples(samples, indices_to_plot)
    PlotAverageSample(samples)
