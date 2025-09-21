# 综合计算工具 / Comprehensive Calculation Tool
---
## 项目介绍
## Project Introduction
这是一个基于MFC（Microsoft Foundation Classes）框架开发的综合计算工具应用程序，提供多种数学计算功能，包括质因数分解、三维三角形计算、线性回归分析和二次方程求解等。
This is a comprehensive calculation tool application developed based on the MFC (Microsoft Foundation Classes) framework, providing various mathematical calculation functions including prime factorization, 3D triangle calculation, linear regression analysis, and quadratic equation solving.

## 功能模块
## Functional Modules

### 1. 质因数分解器
### 1. Prime Factorization Tool
输入一个2到1000000000之间的正整数，程序将自动分解其质因数并以乘法形式显示结果。
Input a positive integer between 2 and 1,000,000,000, and the program will automatically decompose its prime factors and display the result in multiplication form.

### 2. 三维三角形计算器
### 2. 3D Triangle Calculator
输入三维空间中三角形的三个顶点坐标(x1,y1,z1)、(x2,y2,z2)、(x3,y3,z3)，程序将计算并显示三角形的面积和周长。
Input the coordinates of three vertices (x1,y1,z1), (x2,y2,z2), (x3,y3,z3) of a triangle in 3D space, and the program will calculate and display the area and perimeter of the triangle.

### 3. 线性回归分析器
### 3. Linear Regression Analyzer
最多可输入10个二维数据点(x,y)，程序将计算：
You can input up to 10 two-dimensional data points (x,y), and the program will calculate:
- 平均值(\(\bar{x}\), \(\bar{y}\))
- Mean values (\(\bar{x}\), \(\bar{y}\))
- 回归系数k和b (回归方程：y = kx + b)
- Regression coefficients k and b (regression equation: y = kx + b)
- 相关系数r
- Correlation coefficient r
- 可根据输入的x值预测对应的y值
- Predict the corresponding y value based on the input x value

### 4. 二次方程求解器
### 4. Quadratic Equation Solver
支持两种输入方式求解二次方程：
Supports two input methods to solve quadratic equations:
1. 标准式：ax² + bx + c = 0
1. Standard form: ax² + bx + c = 0
2. 顶点式：a(x - h)² + k = 0
2. Vertex form: a(x - h)² + k = 0
程序将计算判别式并显示实根（如果存在）。
The program will calculate the discriminant and display real roots (if they exist).

## 系统要求
## System Requirements
- 操作系统：Windows 7/8/10/11
- Operating System: Windows 7/8/10/11
- 编译器：Visual Studio 2019或更高版本（支持MFC）
- Compiler: Visual Studio 2019 or higher (supporting MFC)
- 架构：x64
- Architecture: x64

## 使用说明
## User Guide
1. 启动应用程序后，会显示主菜单界面
1. After starting the application, the main menu interface will be displayed
2. 点击相应的功能按钮进入对应计算模块
2. Click the corresponding function button to enter the corresponding calculation module
3. 按照界面提示输入相关数据
3. Enter relevant data according to the interface prompts
4. 点击计算按钮获取结果
4. Click the calculate button to get the result
5. 点击返回或关闭按钮返回主菜单
5. Click the back or close button to return to the main menu

## 编译指南
## Compilation Guide
1. 使用Visual Studio打开`综合计算工具.sln`解决方案文件
1. Open the `综合计算工具.sln` solution file with Visual Studio
2. 设置编译配置为`Release`或`Debug`
2. Set the compilation configuration to `Release` or `Debug`
3. 选择目标平台为`x64`
3. Select the target platform as `x64`
4. 点击生成->生成解决方案或按F7编译项目
4. Click Build->Build Solution or press F7 to compile the project
5. 编译完成后，可在`x64/Release`或`x64/Debug`目录下找到可执行文件
5. After compilation is complete, the executable file can be found in the `x64/Release` or `x64/Debug` directory

## 项目结构
## Project Structure
```
综合计算工具/
Comprehensive Calculation Tool/
├── 综合计算工具.cpp          # 应用程序入口文件
├── 综合计算工具.cpp          # Application entry file
├── 综合计算工具.h            # 应用程序头文件
├── 综合计算工具.h            # Application header file
├── 综合计算工具.sln          # 解决方案文件
├── 综合计算工具.sln          # Solution file
├── 综合计算工具.vcxproj      # 项目文件
├── 综合计算工具.vcxproj      # Project file
├── MENU.cpp                  # 主菜单对话框实现
├── MENU.cpp                  # Main menu dialog implementation
├── MENU.h                    # 主菜单对话框头文件
├── MENU.h                    # Main menu dialog header file
├── SEPERATER.cpp             # 质因数分解器实现
├── SEPERATER.cpp             # Prime factorization tool implementation
├── SEPERATER.h               # 质因数分解器头文件
├── SEPERATER.h               # Prime factorization tool header file
├── TRIANGLE.cpp              # 三角形计算器实现
├── TRIANGLE.cpp              # Triangle calculator implementation
├── TRIANGLE.h                # 三角形计算器头文件
├── TRIANGLE.h                # Triangle calculator header file
├── Regression_line.cpp       # 线性回归分析器实现
├── Regression_line.cpp       # Linear regression analyzer implementation
├── Regression_line.h         # 线性回归分析器头文件
├── Regression_line.h         # Linear regression analyzer header file
├── EQUATION.cpp              # 二次方程求解器实现
├── EQUATION.cpp              # Quadratic equation solver implementation
├── EQUATION.h                # 二次方程求解器头文件
├── EQUATION.h                # Quadratic equation solver header file
├── TIP.cpp                   # 提示对话框实现
├── TIP.cpp                   # Tip dialog implementation
├── TIP.h                     # 提示对话框头文件
├── TIP.h                     # Tip dialog header file
├── pch.cpp                   # 预编译头实现
├── pch.cpp                   # Precompiled header implementation
├── pch.h                     # 预编译头文件
├── pch.h                     # Precompiled header file
├── framework.h               # MFC框架头文件
├── framework.h               # MFC framework header file
├── resource.h                # 资源定义头文件
├── resource.h                # Resource definition header file
├── My.rc                     # 资源文件
├── My.rc                     # Resource file
└── res/                      # 资源文件夹
└── res/                      # Resource folder
    └── *.ico                 # 图标文件
    └── *.ico                 # Icon files
```

## 开发说明
## Development Notes
- 项目使用MFC对话框作为界面基础
- The project uses MFC dialog boxes as the interface foundation
- 所有计算功能均封装在独立的对话框类中
- All calculation functions are encapsulated in independent dialog classes
- 使用DDX/DDV机制进行数据交换和验证
- Using DDX/DDV mechanism for data exchange and validation
- 输入验证确保用户输入的数据有效性
- Input validation ensures the validity of user input data
- 采用面向对象的设计方式，便于功能扩展
- Adopting an object-oriented design approach for easy function expansion

## 注意事项
## Notes
1. 输入数据时请确保格式正确，避免非数字字符
1. Please ensure the correct format when entering data and avoid non-numeric characters
2. 质因数分解功能仅支持2到1000000000之间的正整数
2. The prime factorization function only supports positive integers between 2 and 1,000,000,000
3. 线性回归分析至少需要输入2个数据点
3. Linear regression analysis requires at least 2 data points to be input
4. 二次方程求解时请确保输入有效的二次函数
4. Please ensure that a valid quadratic function is input when solving quadratic equations

## 更新日志
## Update Log
- 初始版本：实现所有基础计算功能
- Initial version: Implemented all basic calculation functions
- 优化用户界面和输入验证机制
- Optimized user interface and input validation mechanism
- 完善错误处理和提示信息
- Improved error handling and prompt messages

## 许可证
## License
本项目为学习用途开发，仅供个人学习和参考使用。
This project is developed for learning purposes and is for personal study and reference only.