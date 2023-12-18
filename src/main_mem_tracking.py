import psutil
import time

def track_memory_usage(file_path, duration_sec):
    with open(file_path, 'w') as f:
        start_time = time.time()
        end_time = start_time + duration_sec

        f.write("Time (s),Memory Usage (MB),Memory Usage (%)\n")

#         while time.time() < end_time:
        while 1:
            memory_usage = psutil.virtual_memory().used / (1024 ** 2)  # Convert to MB
            memory_percentage = psutil.virtual_memory()[2]
            elapsed_time = time.time() - start_time

            f.write(f"{elapsed_time:.2f},{memory_usage:.2f},{memory_percentage:.2f}\n")

            time.sleep(1)

if __name__ == "__main__":
    file_path = "memory_usage.csv"
    duration_sec = 20  # Adjust the duration as needed

    track_memory_usage(file_path, duration_sec)
    print(f"Memory usage data saved to {file_path}")
