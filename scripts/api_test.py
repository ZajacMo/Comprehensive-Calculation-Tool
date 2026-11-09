#!/usr/bin/env python3
"""端到端 API 测试：拉起服务 → 逐个调用 /api/* 端点 → 校验返回的数值。

用法：python scripts/api_test.py [端口]
默认端口 8099。若该端口已有服务在跑，则直接复用。
"""
import json
import math
import os
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request

PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 8099
BASE = f"http://127.0.0.1:{PORT}"

failures = []
checks = 0
server_process = None


def isServerUp():
    """服务是否已经在运行。"""
    try:
        with urllib.request.urlopen(f"{BASE}/api/health", timeout=2) as response:
            return response.status == 200
    except Exception:
        return False


def startServer():
    """若服务未运行，则启动一个。返回进程对象（可能为 None，表示复用了已有服务）。"""
    global server_process
    if isServerUp():
        print("检测到已在运行的服务，直接复用。")
        return None

    # 优先用构建目录里的可执行文件。
    candidates = [
        os.path.join("build", "calc_server.exe"),
        os.path.join("build", "calc_server"),
        os.path.join("..", "build", "calc_server.exe"),
        os.path.join("..", "build", "calc_server"),
    ]
    executable = next((c for c in candidates if os.path.exists(c)), None)
    if executable is None:
        print("找不到 calc_server 可执行文件，请先构建。")
        sys.exit(2)

    print(f"启动服务：{executable} --port {PORT}")
    server_process = subprocess.Popen(
        [executable, "--port", str(PORT), "--no-browser"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )

    # 等待服务就绪。
    for _ in range(50):
        time.sleep(0.2)
        if isServerUp():
            print("服务已就绪。")
            return server_process
    print("服务启动超时。")
    server_process.terminate()
    sys.exit(2)


def stopServer():
    if server_process is not None:
        server_process.terminate()
        try:
            server_process.wait(timeout=10)
        except subprocess.TimeoutExpired:
            server_process.kill()


def call(path, **params):
    """调用 API 并返回解析后的 JSON。"""
    query = urllib.parse.urlencode(params)
    url = f"{BASE}{path}" + (f"?{query}" if query else "")
    with urllib.request.urlopen(url, timeout=15) as response:
        return json.loads(response.read().decode("utf-8"))


def check(label, actual, expected, tolerance=1e-9):
    """校验一个数值或字符串是否满足预期。"""
    global checks
    checks += 1
    if isinstance(expected, float) or isinstance(actual, float):
        try:
            ok = math.isclose(float(actual), float(expected), rel_tol=tolerance, abs_tol=tolerance)
        except (TypeError, ValueError):
            ok = False
    else:
        ok = actual == expected
    if not ok:
        failures.append(f"{label}: 期望 {expected!r}，实际 {actual!r}")
        print(f"  FAIL {label}: 期望 {expected!r}，实际 {actual!r}")
    else:
        print(f"  ok   {label} = {actual!r}")


def section(name):
    print(f"\n[{name}]")


def main():
    startServer()
    try:
        return runChecks()
    finally:
        stopServer()


def runChecks():
    section("健康检查")
    health = call("/api/health")
    check("status", health["status"], "ok")
    check("version 非空", bool(health["version"]), True)

    section("质因数分解")
    data = call("/api/factorize", n=1000000000)
    check("分解文本", data["factorizationText"], "2^9 * 5^9")
    check("欧拉函数", data["eulerPhi"], 400000000)
    check("莫比乌斯", data["mobius"], 0)
    check("约数个数", data["divisorCount"], 100)
    check("约数和", data["divisorSum"], 2497558338)
    check("十六进制", data["base16"], "3B9ACA00")
    check("分解自洽", data["reconstructs"], True)

    data = call("/api/isprime", n=561)
    check("561 不是素数", data["isPrime"], False)

    section("进制转换")
    data = call("/api/base-convert", value="FF", base=16)
    check("十六进制 FF", data["value"], 255)
    check("二进制", data["base2"], "11111111")
    check("十进制", data["base10"], "255")

    section("模运算")
    data = call("/api/modular", a=3, m=11)
    check("gcd", data["gcd"], 1)
    check("逆元", data["inverse"], 4)
    check("裴蜀验证", data["a"] * data["bezoutX"] + data["m"] * data["bezoutY"], 1)

    section("描述统计")
    data = call("/api/stats", values="1,2,3,4,5")
    check("样本数", data["count"], 5)
    check("均值", data["mean"], 3.0)
    check("中位数", data["median"], 3.0)
    check("样本方差", data["variance"], 2.5)
    check("标准差", data["stdDev"], math.sqrt(2.5))
    check("极差", data["range"], 4.0)

    section("线性回归")
    data = call("/api/regress", points="1,2,2,4,3,6")
    check("斜率 k", data["slope"], 2.0)
    check("截距 b", data["intercept"], 0.0)
    check("相关系数 r", data["r"], 1.0)
    check("决定系数", data["rSquared"], 1.0)

    data = call("/api/regress", points="1,2.1,2,3.9,3,6.2,4,7.8", x=5)
    check("预测 x=5 时 y", data["predictY"], data["slope"] * 5 + data["intercept"])

    section("方程求解")
    data = call("/api/quadratic", a=1, b=-5, c=6)
    check("判别式", data["discriminant"], 1.0)
    check("有实根", data["hasRealRoots"], True)
    check("根 1", data["roots"][0]["real"], 3.0)
    check("根 2", data["roots"][1]["real"], 2.0)

    data = call("/api/quadratic", a=1, b=0, c=1)
    check("无实根", data["hasRealRoots"], False)
    check("复根模", abs(data["roots"][0]["imag"]), 1.0)

    data = call("/api/vertex", a=1, h=2, k=-9)
    roots = sorted(r["real"] for r in data["roots"])
    check("顶点式根 1", roots[0], -1.0)
    check("顶点式根 2", roots[1], 5.0)

    data = call("/api/linear", a=2, b=-4)
    check("一次方程根", data["roots"][0]["real"], 2.0)

    data = call("/api/polynomial", coefficients="1,-6,11,-6")
    check("三次方程根数", data["count"], 3)
    real_roots = sorted(r["real"] for r in data["roots"] if abs(r["imag"]) < 1e-6)
    check("三次方程根", real_roots, [1.0, 2.0, 3.0])

    section("矩阵")
    data = call("/api/matrix", rows=2, cols=2, values="1,2,3,4")
    check("行列式", data["determinant"], -2.0)
    check("秩", data["rank"], 2)
    check("可逆", data["invertible"], True)
    check("逆矩阵 [0][0]", data["inverse"][0], -2.0)
    check("逆矩阵 [0][1]", data["inverse"][1], 1.0)

    section("线性方程组")
    data = call("/api/solve-system", a="1,1,1,-1", b="3,1")
    check("唯一解", data["hasInfiniteSolutions"], False)
    check("解 x", data["solution"][0], 2.0)
    check("解 y", data["solution"][1], 1.0)

    section("几何")
    data = call("/api/triangle", points="0,0,0,3,0,0,0,4,0")
    check("面积", data["area"], 6.0)
    check("周长", data["perimeter"], 12.0)
    check("退化标记", data["degenerate"], False)

    data = call("/api/triangle", points="0,0,0,1,0,0,2,0,0")
    check("退化检测", data["degenerate"], True)

    data = call("/api/tetrahedron", points="0,0,0,1,0,0,0,1,0,0,0,1")
    check("体积", data["volume"], 1.0 / 6.0)
    check("重心 x", data["centroid"][0], 0.25)

    data = call("/api/solid", kind="sphere", radius=2)
    check("球表面积", data["surfaceArea"], 4 * math.pi * 4)
    check("球体积", data["volume"], (4.0 / 3.0) * math.pi * 8)

    data = call("/api/solid", kind="cylinder", radius=2, height=5)
    check("圆柱体积", data["volume"], math.pi * 4 * 5)

    data = call("/api/solid", kind="prism", radius=1, height=2, sides=6)
    check("正六棱柱体积", data["volume"], 3 * math.sqrt(3) / 2 * 2)

    section("数列与组合")
    data = call("/api/sequences", n=10)
    check("斐波那契 F(10)", data["fibonacci"], 55)
    check("卢卡斯 L(10)", data["lucas"], 123)
    check("卡特兰数 C(10)", data["catalan"], 16796)
    check("阶乘 10!", data["factorial"], 3628800)
    check("分拆数 p(10)", data["partition"], 42)

    data = call("/api/sequences", n=5)
    check("卡特兰数 C(5)", data["catalan"], 42)
    check("阶乘 5!", data["factorial"], 120)

    data = call("/api/combinatorics", n=10, k=3)
    check("组合数 C(10,3)", data["binomial"], 120)
    check("排列数 A(10,3)", data["permutation"], 720)

    section("复数")
    data = call("/api/complex", real=1, imag=1, power=2, roots=3)
    check("模", data["modulus"], math.sqrt(2))
    check("平方", data["powerReal"], 0.0)
    check("平方虚部", data["powerImag"], 2.0)
    check("三次根个数", len(data["roots"]), 3)

    section("静态资源")
    with urllib.request.urlopen(f"{BASE}/", timeout=15) as response:
        html = response.read().decode("utf-8")
    check("首页标题", "综合计算工具" in html, True)
    with urllib.request.urlopen(f"{BASE}/app.js", timeout=15) as response:
        js = response.read().decode("utf-8")
    check("前端脚本非空", len(js) > 1000, True)
    with urllib.request.urlopen(f"{BASE}/styles.css", timeout=15) as response:
        css = response.read().decode("utf-8")
    check("样式表非空", len(css) > 500, True)

    section("错误处理")
    try:
        call("/api/factorize", n=1)
        check("非法输入应报错", False, True)
    except urllib.error.HTTPError as error:
        payload = json.loads(error.read().decode("utf-8"))
        check("错误状态码", error.code, 400)
        check("错误消息非空", bool(payload["error"]), True)

    try:
        urllib.request.urlopen(f"{BASE}/api/nonexistent", timeout=15)
        check("未知路由应 404", False, True)
    except urllib.error.HTTPError as error:
        check("未知路由状态码", error.code, 404)

    print("\n" + "-" * 50)
    if failures:
        print(f"失败 {len(failures)} / {checks} 项检查：")
        for failure in failures:
            print(f"  - {failure}")
        return 1
    print(f"全部 {checks} 项检查通过。")
    return 0


if __name__ == "__main__":
    sys.exit(main())
