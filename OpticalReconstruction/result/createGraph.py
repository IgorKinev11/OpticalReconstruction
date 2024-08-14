import matplotlib.pyplot as plt

def read_data(file_path):
    with open(file_path, 'r') as file:
        data = [float(line.strip()) for line in file]

    grouped_data = [data[i:i+3] for i in range(0, len(data), 3)]
    return grouped_data

def plot_data(data):
    iterations = len(data)
    labels = [f'BRS {i}' for i in range(len(data[0]))]
    markers = ['o', 's', '^', 'D', 'x', '*']
    
    for i in range(len(data[0])):
        values = [iteration[i] for iteration in data]
        plt.plot(range(1, iterations+1), values, marker=markers[i % len(markers)], label=labels[i])
    
    plt.xlabel('Iteration')
    plt.ylabel('Value')
    plt.title('Values per Iteration')
    plt.legend()
    plt.grid(True)
    plt.savefig(file_path + '_graph.jpg')
    plt.show()

file_path = 'graph.txt'
data = read_data(file_path)
plot_data(data)
