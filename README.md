This program is designed to compare the delay between two keyboards. The tested keyboard will use the Y key for the test, and the reference keyboard will use the C key. Each time a key is pressed or released, you must press the D key and release it once to record this time difference. If you accidentally press something unintentionally, you can press the R key to reset the values stored in the program. This will only reset values that have not been sent with the D key.

This program will calculate the average press and release latency, eliminating outliers from the test. It will remove the maximum 5 values and minimum 5 values, you should test at least 50 times for accurate results.

If the measured values are negative, it indicates that the tested keyboard has a faster response than the reference keyboard.

![alt text](https://github.com/SharpKunG1412/keyboard-delta-ms-tester/blob/main/timing_diagram.png?raw=true)
