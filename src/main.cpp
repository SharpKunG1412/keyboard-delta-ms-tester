#include <GLFW/glfw3.h>

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>
#include <math.h>

#define TEST_POLLING_RATE 2 // 0 = don't. 1 = old mode, instant console output, creates issues with simultaneous keypresses. 2 = new mode, console output batched until end.
// currently, keyboard polling and mouse polling are mixed together, so only move your keyboard or your mouse, but not both simultaneously

using frame_clock = std::chrono::steady_clock; // high resolution clock uses an unfortunate realtime clock on Linux https://stackoverflow.com/a/41203433/
using float_millisecond = std::chrono::duration<float, std::ratio<1, 1000>>;
GLFWwindow *window;
std::vector<frame_clock::time_point> input_times;
std::vector<char> input_key;
std::vector<char> input_state;
float delta_avg = {0.0};
float raw[1000] = {0.0};
float stdr = 0.0;
float abso = 0.0;
int key_count = {-10};
float avoid_min[5] = {10000.0}; // remove 5 maximum and 5 minimum value to minimize error during measurement
float avoid_max[5] = {-10000.0};
float prepareminmax[10];
bool interlock = {false}; // avoid from double press after C key is set as ref_time

float delta_avg2 = {0.0};
float raw2[1000] = {0.0};
float stdr2 = 0.0;
float abso2 = 0.0;
int key_count2 = {-10};
float avoid_min2[5] = {10000.0}; // remove 5 maximum and 5 minimum value to minimize error during measurement
float avoid_max2[5] = {-10000.0};
float prepareminmax2[10];
bool interlock2 = {false}; // avoid from double press after C key is set as ref_time

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (TEST_POLLING_RATE == 1 && action == GLFW_PRESS)
	{
		static frame_clock::time_point previous_time = frame_clock::now();
		auto now = frame_clock::now();
		std::cout << (float_millisecond(now - previous_time).count()) << ' ';
		previous_time = now;
	}
	else if (TEST_POLLING_RATE == 2 && action == GLFW_PRESS)
	{
		input_times.push_back(frame_clock::now());
		input_key.push_back(key);
		input_state.push_back('v');
	}
	else if (TEST_POLLING_RATE == 2 && action == GLFW_RELEASE)
	{
		input_times.push_back(frame_clock::now());
		input_key.push_back(key);
		input_state.push_back('^');
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (TEST_POLLING_RATE == 1)
	{
		static frame_clock::time_point previous_time = frame_clock::now();
		auto now = frame_clock::now();
		std::cout << (float_millisecond(now - previous_time).count()) << ' ';
		previous_time = now;
	}
	else if (TEST_POLLING_RATE == 2)
	{
		input_times.push_back(frame_clock::now());
		input_key.push_back('m');
		input_state.push_back(' ');
	}
}

void input_loop()
{
	frame_clock::time_point ref_time, ref_time2;
	ref_time = frame_clock::now();
	ref_time2 = frame_clock::now();
	while (!glfwWindowShouldClose(window))
	{
		if (1)
		{
			// pollevents option
			glfwPollEvents();
			// std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (!TEST_POLLING_RATE)
				std::this_thread::yield(); // causes a whole bunch of CPU usage! when compared to sleeping or WaitEvents
		}
		else
			glfwWaitEvents(); // this is actually quite good. not sure if it's as good as constant polling

		if (TEST_POLLING_RATE == 2)
		{

			if (input_times.size() > 0 && (input_key[input_times.size() - 1] == ' ' || input_key[input_times.size() - 1] == 'D' ))
			{
				for (int x = 1; x < input_times.size(); ++x)
				{
					if (input_key[x] == 'C' && input_state[x] == 'v')
					{
						ref_time = input_times[x];
						for (int i = 0; i < 26; i++)
							interlock = true;
						break;
					}
				}
				for (int x = 1; x < input_times.size(); ++x)
				{
					if ((input_key[x] == 'G' || input_key[x] == 'H' || input_key[x] == 'T' || input_key[x] == 'Y') && input_state[x] == 'v')
					{
						if (interlock == true)
						{
							interlock = false;
							float result = float_millisecond(input_times[x] - ref_time).count();
							float mem, result_b = result;
							// check is new min or new max
							if (result > -500.0 && result < 500.0)
							{
								if(key_count < 0)
								{
									prepareminmax[key_count+10] = result;
								}
								else
								{
									if(key_count == 0)
									{
										for(int i = 9; i >= 0; --i)
										{
											for(int j = 0; j < i; ++j)
											{
												if(prepareminmax[j] > prepareminmax[j+1])
												{
													mem = prepareminmax[j];
													prepareminmax[j] = prepareminmax[j+1];
													prepareminmax[j+1] = mem;
												}
											}
										}
										for(int i = 0; i < 5; ++i)
										{
											avoid_min[4-i] = prepareminmax[i];
											avoid_max[4-i] = prepareminmax[9-i];
										}
									}
									if (result > avoid_max[0])
									{
										mem = result;
										result = avoid_max[0];
										avoid_max[0] = mem;
										for(int i = 0; i < 4; ++i)
										{
											if (avoid_max[i] > avoid_max[i+1])
											{
												mem = avoid_max[i];
												avoid_max[i] = avoid_max[i+1];
												avoid_max[i+1] = mem;
											}
										}
									}
									else if (result < avoid_min[0])
									{
										mem = result;
										result = avoid_min[0];
										avoid_min[0] = mem;
										for(int i = 0; i < 4; ++i)
										{
											if (avoid_min[i] < avoid_min[i+1])
											{
												mem = avoid_min[i];
												avoid_min[i] = avoid_min[i+1];
												avoid_min[i+1] = mem;
											}
										}
									}
									if(result > -9999 && result < 9999)
									{
										delta_avg = (delta_avg * (float)key_count + result)/float(key_count+1);
										if(key_count > 500)
											return;
										raw[key_count] = result;
									}
								}
								key_count++;

								stdr = 0.0;
								for (int s = 0; s < key_count; s++)
								{
									abso = raw[s] - delta_avg;
									stdr += abso * abso;
								}
								std::cout << "\n";
								std::cout << "----- press -----\n";
								std::cout << "minimum that reject " << avoid_min[0] << " " << avoid_min[1] << " " << avoid_min[2] << " " << avoid_min[3] << " " << avoid_min[4] << '\n';
								std::cout << "maximum that reject " << avoid_max[0] << " " << avoid_max[1] << " " << avoid_max[2] << " " << avoid_max[3] << " " << avoid_max[4] << '\n';
								std::cout << "curr " << result_b << " | " << " sel " << result << '\n';
								std::cout << "avg " << delta_avg << " | key_count " << key_count << '\n';
								if(key_count > 1)
									std::cout << "std " << sqrt(stdr/(key_count)) << '\n';
							}
						}
					}
				}
				
				for (int x = 1; x < input_times.size(); ++x)
				{
					if (input_key[x] == 'C' && input_state[x] == '^')
					{
						ref_time2 = input_times[x];
						for (int i = 0; i < 26; i++)
							interlock2 = true;
						break;
					}
				}
				for (int x = 1; x < input_times.size(); ++x)
				{
					if ((input_key[x] == 'G' || input_key[x] == 'H' || input_key[x] == 'T' || input_key[x] == 'Y') && input_state[x] == '^')
					{
						if (interlock2 == true)
						{
							interlock2 = false;
							float result = float_millisecond(input_times[x] - ref_time2).count();
							float mem, result_b = result;
							// check is new min or new max
							if (result > -500.0 && result < 500.0)
							{
								if(key_count2 < 0)
								{
									prepareminmax2[key_count2+10] = result;
								}
								else
								{
									if(key_count2 == 0)
									{
										for(int i = 9; i >= 0; --i)
										{
											for(int j = 0; j < i; ++j)
											{
												if(prepareminmax2[j] > prepareminmax2[j+1])
												{
													mem = prepareminmax2[j];
													prepareminmax2[j] = prepareminmax2[j+1];
													prepareminmax2[j+1] = mem;
												}
											}
										}
										for(int i = 0; i < 5; ++i)
										{
											avoid_min2[4-i] = prepareminmax2[i];
											avoid_max2[4-i] = prepareminmax2[9-i];
										}
									}
									if (result > avoid_max2[0])
									{
										mem = result;
										result = avoid_max2[0];
										avoid_max2[0] = mem;
										for(int i = 0; i < 4; ++i)
										{
											if (avoid_max2[i] > avoid_max2[i+1])
											{
												mem = avoid_max2[i];
												avoid_max2[i] = avoid_max2[i+1];
												avoid_max2[i+1] = mem;
											}
										}
									}
									else if (result < avoid_min2[0])
									{
										mem = result;
										result = avoid_min2[0];
										avoid_min2[0] = mem;
										for(int i = 0; i < 4; ++i)
										{
											if (avoid_min2[i] < avoid_min2[i+1])
											{
												mem = avoid_min2[i];
												avoid_min2[i] = avoid_min2[i+1];
												avoid_min2[i+1] = mem;
											}
										}
									}
									if(result > -9999 && result < 9999)
									{
										delta_avg2 = (delta_avg2 * (float)key_count2 + result)/float(key_count2+1);
										if(key_count2 > 500)
											return;
										raw2[key_count2] = result;
									}
								}
								key_count2++;

								stdr2 = 0.0;
								for (int s = 0; s < key_count2; s++)
								{
									abso2 = raw2[s] - delta_avg2;
									stdr2 += abso2 * abso2;
								}
								std::cout << "----- release -----\n";
								std::cout << "minimum that reject " <<  avoid_min2[0] << " " << avoid_min2[1] << " " << avoid_min2[2] << " " << avoid_min2[3] << " " << avoid_min2[4] << '\n';
								std::cout << "maximum that reject " << avoid_max2[0] << " " << avoid_max2[1] << " " << avoid_max2[2] << " " << avoid_max2[3] << " " << avoid_max2[4] << '\n';
								std::cout << "curr " << result_b << " | " << " sel " << result << '\n';
								std::cout << "avg " << delta_avg2 << " | key_count " << key_count2 << '\n';
								if(key_count2 > 1)
									std::cout << "std " << sqrt(stdr2/(key_count2)) << '\n';
								std::cout << "\n";
							}
						}
					}
				}
				if (input_times.size() > 1)
				{
					input_times = {input_times.back()};
					input_key = {input_key.back()};
					input_state = {input_state.back()};
				}
			}
			else if (input_times.size() > 0 && (input_key[input_times.size() - 1] == 'R'))
			{
				input_times = {input_times.back()};
				input_key = {input_key.back()};
				input_state = {input_state.back()};
			}
		}
	}
}

int main()
{
	for(int i = 0; i < 5; ++i)
	{
		avoid_min[i] = 10000.0;
		avoid_max[i] = -10000.0;
		avoid_min2[i] = 10000.0;
		avoid_max2[i] = -10000.0;
	}	
			
	glfwInit();

	window = glfwCreateWindow(800, 600, "input test", nullptr, nullptr);

	if (window == NULL)
	{
		std::cout << ("Failed to create GLFW window");
		glfwTerminate();
		return -1;
	}
	glfwSetCursorPosCallback(window, mouse_cursor_callback);
	glfwSetKeyCallback(window, key_callback);

	input_loop();

	glfwTerminate();
	return 0;
}
