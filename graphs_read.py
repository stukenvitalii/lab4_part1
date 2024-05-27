import os
import matplotlib.pyplot as plt


# Функция для построения графика
def plot_graph(states, times, filename):
    plt.figure(figsize=(10, 6))
    plt.plot(times, states, marker='o')
    plt.xlabel('Time')
    plt.ylabel('State')
    plt.title('State vs Time')
    plt.grid(True)
    plt.savefig(filename)
    plt.close()


# Перебор файлов
for i in range(5):
    filename = f"D:/tanki/University/OS/lab4/cmake-build-debug/logs/readlogs/readLog #{i}.txt"
    states = []
    times = []

    if os.path.exists(filename):
        with open(filename, 'r') as file:
            for line in file:
                tokens = line.strip().split(' | ')
                if len(tokens) < 3:
                    continue
                states.append(tokens[0])
                times.append(int(tokens[2]))

        plot_filename = f"D:/tanki/University/OS/lab4/graphs/graph_read_{i}.png"
        plot_graph(states, times, plot_filename)
        print(f"График для {filename} создан как {plot_filename}")
    else:
        print(f"Файл {filename} не найден")

print("Готово.")
