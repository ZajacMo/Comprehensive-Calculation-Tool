/* 综合计算工具 · Web 前端
 *
 * 原生 ES6，无构建步骤、无外部依赖。所有计算通过 /api/* 转发到本地 C++ 内核。
 */
'use strict';

// ---------------------------------------------------------------- 工具函数

/** 调用 API 并返回解析后的 JSON。 */
async function api(path, params) {
    const query = new URLSearchParams();
    for (const [key, value] of Object.entries(params || {})) {
        if (value === undefined || value === null || value === '') continue;
        query.append(key, value);
    }
    const url = query.toString() ? `${path}?${query}` : path;

    const response = await fetch(url);
    const text = await response.text();
    let payload;
    try {
        payload = JSON.parse(text);
    } catch (error) {
        throw new Error(`服务返回了非 JSON 内容（HTTP ${response.status}）`);
    }
    if (!response.ok || payload.ok === false) {
        throw new Error(payload.error || `请求失败（HTTP ${response.status}）`);
    }
    return payload;
}

/** 把数字格式化为便于阅读的字符串。 */
function fmt(value, digits = 6) {
    if (value === null || value === undefined) return '—';
    if (typeof value !== 'number') return String(value);
    if (!Number.isFinite(value)) return '—';
    if (Number.isInteger(value) && Math.abs(value) < 1e15) return String(value);
    const fixed = value.toFixed(digits);
    // 去掉末尾多余的 0。
    return fixed.replace(/\.?0+$/, '');
}

/** 复数的可读形式。 */
function fmtComplex(root) {
    if (!root) return '—';
    const re = root.real ?? 0;
    const im = root.imag ?? 0;
    if (Math.abs(im) < 1e-12) return fmt(re);
    const sign = im < 0 ? ' − ' : ' + ';
    return `${fmt(re)}${sign}${fmt(Math.abs(im))}i`;
}

/** 创建元素。 */
function el(tag, attrs, children) {
    const node = document.createElement(tag);
    for (const [key, value] of Object.entries(attrs || {})) {
        if (key === 'class') node.className = value;
        else if (key === 'html') node.innerHTML = value;
        else if (key === 'text') node.textContent = value;
        else if (key.startsWith('on') && typeof value === 'function') {
            node.addEventListener(key.slice(2).toLowerCase(), value);
        } else if (value !== undefined && value !== null) {
            node.setAttribute(key, value);
        }
    }
    for (const child of [].concat(children || [])) {
        if (child === null || child === undefined) continue;
        node.appendChild(typeof child === 'string' ? document.createTextNode(child) : child);
    }
    return node;
}

/** 结果面板：负责展示键值结果。 */
class ResultPanel {
    constructor() {
        this.rows = [];
        this.root = el('div', { class: 'result' });
        this.head = el('div', { class: 'result-head' }, [
            el('span', { text: '计算结果' }),
            el('button', {
                text: '复制',
                onclick: () => this.copy(),
            }),
        ]);
        this.body = el('div', { class: 'result-body' });
        this.root.append(this.head, this.body);
        this.render();
    }

    set(key, value, highlight = false) {
        this.rows = this.rows.filter((row) => row.key !== key);
        this.rows.push({ key, value, highlight });
        this.render();
    }

    setMany(entries) {
        this.rows = [];
        for (const [key, value, highlight] of entries) {
            this.rows.push({ key, value, highlight });
        }
        this.render();
    }

    clear() {
        this.rows = [];
        this.render();
    }

    render() {
        this.body.innerHTML = '';
        if (this.rows.length === 0) {
            this.body.appendChild(el('div', { class: 'result-empty', text: '暂无结果' }));
            return;
        }
        for (const row of this.rows) {
            const valueClass = row.highlight ? 'result-value highlight' : 'result-value';
            this.body.appendChild(el('div', { class: 'result-row' }, [
                el('div', { class: 'result-key', text: row.key }),
                el('div', { class: valueClass, text: row.value }),
            ]));
        }
    }

    async copy() {
        const text = this.rows.map((row) => `${row.key}: ${row.value}`).join('\n');
        try {
            await navigator.clipboard.writeText(text);
            const button = this.head.querySelector('button');
            const original = button.textContent;
            button.textContent = '已复制';
            setTimeout(() => { button.textContent = original; }, 1200);
        } catch (error) {
            console.warn('复制失败', error);
        }
    }
}

/** 状态提示条。 */
class Message {
    constructor() {
        this.root = el('div', { class: 'message' });
    }

    info(text) {
        this.root.className = 'message show info';
        this.root.textContent = text;
    }

    error(text) {
        this.root.className = 'message show error';
        this.root.textContent = text;
    }

    clear() {
        this.root.className = 'message';
        this.root.textContent = '';
    }
}

/** 生成一个输入框字段。 */
function field(label, key, options = {}) {
    let input;
    if (options.type === 'select') {
        input = el('select', { id: `f-${key}` });
        for (const [value, text] of options.options || []) {
            input.appendChild(el('option', { value, text }));
        }
        if (options.value !== undefined) input.value = options.value;
    } else {
        input = el('input', {
            type: options.type || 'text',
            id: `f-${key}`,
            placeholder: options.placeholder || '',
            value: options.value === undefined ? '' : options.value,
            step: options.step || 'any',
            inputmode: options.inputmode || (options.type === 'number' ? 'decimal' : undefined),
        });
    }
    return {
        node: el('div', { class: options.wide ? 'field wide' : 'field' }, [
            el('label', { for: `f-${key}`, text: label }),
            input,
        ]),
        input,
    };
}

/** 从容器中收集所有输入框的值。 */
function collect(container) {
    const values = {};
    for (const input of container.querySelectorAll('input, select')) {
        values[input.id.replace(/^f-/, '')] = input.value.trim();
    }
    return values;
}

// ---------------------------------------------------------------- 页面定义

/** 通用页面构造：标题 + 说明 + 若干面板。 */
function page(title, description) {
    const root = el('div', { class: 'page' });
    root.appendChild(el('h2', { class: 'page-title', text: title }));
    if (description) {
        root.appendChild(el('p', { class: 'page-desc', text: description }));
    }
    return root;
}

/** 创建一个带按钮与结果面板的面板。 */
function panel(title, fields, onCalculate) {
    const root = el('div', { class: 'panel' });
    if (title) root.appendChild(el('h3', { class: 'panel-title', text: title }));

    const grid = el('div', { class: 'field-grid' });
    const inputs = {};
    for (const spec of fields) {
        const built = field(spec.label, spec.key, spec);
        inputs[spec.key] = built.input;
        grid.appendChild(built.node);
    }
    root.appendChild(grid);

    const message = new Message();
    const results = new ResultPanel();
    const button = el('button', { class: 'primary', text: '计算' });

    const run = async () => {
        button.disabled = true;
        message.clear();
        results.clear();
        try {
            await onCalculate(collect(root), results, message);
        } catch (error) {
            message.error(error.message || String(error));
        } finally {
            button.disabled = false;
        }
    };

    button.addEventListener('click', run);
    for (const input of Object.values(inputs)) {
        input.addEventListener('keydown', (event) => {
            if (event.key === 'Enter') run();
        });
    }

    const actions = el('div', { class: 'actions' }, [button]);
    root.append(actions, message.root, results.root);
    return { root, results, message, inputs, run, fillFromParams };
}

/** 用 URL 查询参数填充输入框。返回是否填充了任何内容。
 *  params 为 URLSearchParams。 */
function fillFromParams(inputs, params) {
    let filled = false;
    for (const [key, input] of Object.entries(inputs)) {
        if (!params.has(key)) continue;
        const value = params.get(key);
        if (input && typeof input === 'object' && 'value' in input) {
            input.value = value;
        } else if (input) {
            input.value = value;
        }
        filled = true;
    }
    return filled;
}

// ---------------------------------------------------------------- 各功能页

const PAGES = [
    {
        id: 'factorize',
        icon: '⑂',
        title: '质因数分解',
        build() {
            const root = page('质因数分解', '输入一个不小于 2 的整数，得到质因数分解、欧拉函数 φ、莫比乌斯函数 μ 与约数统计。分解只试到 √n，10⁹ 级别也能即时返回。');
            const p = panel('输入', [
                { label: '整数 n', key: 'n', placeholder: '例如 1000000000', type: 'number' },
            ], async (values, results) => {
                const data = await api('/api/factorize', { n: values.n });
                results.setMany([
                    ['输入值 n', fmt(data.n)],
                    ['质因数分解', data.factorizationText, true],
                    ['是否素数', data.isPrime ? '是' : '否'],
                    ['欧拉函数 φ(n)', fmt(data.eulerPhi)],
                    ['莫比乌斯函数 μ(n)', fmt(data.mobius)],
                    ['正约数个数', fmt(data.divisorCount)],
                    ['正约数之和', fmt(data.divisorSum)],
                    ['二进制', data.base2],
                    ['八进制', data.base8],
                    ['十六进制', data.base16],
                ]);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'base',
        icon: '⇄',
        title: '进制转换',
        build() {
            const root = page('进制转换', '在 2 到 36 进制之间转换整数，同时给出模逆元与裴蜀系数。');
            const p = panel('输入', [
                { label: '数值（按所选进制书写）', key: 'value', placeholder: '例如 FF' },
                { label: '源进制', key: 'base', value: '16', type: 'number' },
            ], async (values, results) => {
                const data = await api('/api/base-convert', { value: values.value, base: values.base });
                results.setMany([
                    ['十进制', fmt(data.value), true],
                    ['二进制', data.base2],
                    ['八进制', data.base8],
                    ['十六进制', data.base16],
                    ['三十六进制', data.base36],
                ]);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'modular',
        icon: '≡',
        title: '模运算',
        build() {
            const root = page('模运算', '计算最大公约数、模逆元与裴蜀等式系数。');
            const p = panel('输入', [
                { label: '整数 a', key: 'a', placeholder: '例如 3', type: 'number' },
                { label: '模数 m', key: 'm', placeholder: '例如 11', type: 'number' },
            ], async (values, results) => {
                const data = await api('/api/modular', { a: values.a, m: values.m });
                results.setMany([
                    ['最大公约数 gcd(a, m)', fmt(data.gcd), true],
                    ['模逆元', data.hasInverse ? fmt(data.inverse) : '不存在（gcd ≠ 1）'],
                    ['裴蜀系数 x', fmt(data.bezoutX)],
                    ['裴蜀系数 y', fmt(data.bezoutY)],
                    ['验证 a·x + m·y', fmt(data.a * data.bezoutX + data.m * data.bezoutY)],
                ]);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'stats',
        icon: '∑',
        title: '描述统计',
        build() {
            const root = page('描述统计', '输入一组数值，得到均值、中位数、方差、标准差、分位数与四分位距。');
            const p = panel('输入', [
                { label: '数值（逗号分隔）', key: 'values', placeholder: '例如 1, 2, 3, 4, 5', wide: true },
            ], async (values, results) => {
                const data = await api('/api/stats', { values: values.values });
                results.setMany([
                    ['样本数 n', fmt(data.count)],
                    ['平均值', fmt(data.mean), true],
                    ['中位数', fmt(data.median)],
                    ['方差（样本）', fmt(data.variance)],
                    ['标准差', fmt(data.stdDev)],
                    ['最小值', fmt(data.min)],
                    ['最大值', fmt(data.max)],
                    ['极差', fmt(data.range)],
                    ['第一四分位 Q1', fmt(data.q1)],
                    ['第三四分位 Q3', fmt(data.q3)],
                    ['四分位距 IQR', fmt(data.iqr)],
                    ['总和', fmt(data.sum)],
                ]);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'regress',
        icon: '⟋',
        title: '线性回归',
        build() {
            const root = page('线性回归', '录入若干 (x, y) 数据点，得到最小二乘拟合直线的斜率 k、截距 b、相关系数 r 与决定系数 R²，并可代入 x 预测 y。');

            const panelNode = el('div', { class: 'panel' });
            panelNode.appendChild(el('h3', { class: 'panel-title', text: '数据点' }));

            const table = el('table', { class: 'points-table' });
            const head = el('tr', {}, [
                el('th', { text: '点号' }), el('th', { text: 'x' }), el('th', { text: 'y' }),
            ]);
            const body = el('tbody');
            table.append(head, body);

            let rowCount = 6;
            function renderRows() {
                body.innerHTML = '';
                for (let i = 0; i < rowCount; i++) {
                    body.appendChild(el('tr', {}, [
                        el('td', { text: String(i + 1) }),
                        el('td', {}, [el('input', { type: 'number', step: 'any', 'data-x': i })]),
                        el('td', {}, [el('input', { type: 'number', step: 'any', 'data-y': i })]),
                    ]));
                }
            }
            renderRows();

            const addButton = el('button', { text: '增加一行' });
            const removeButton = el('button', { text: '删除末行' });
            const clearButton = el('button', { text: '清空' });
            addButton.addEventListener('click', () => { rowCount++; renderRows(); });
            removeButton.addEventListener('click', () => { if (rowCount > 1) { rowCount--; renderRows(); } });
            clearButton.addEventListener('click', () => {
                for (const input of body.querySelectorAll('input')) input.value = '';
            });

            panelNode.appendChild(table);
            panelNode.appendChild(el('div', { class: 'actions' }, [addButton, removeButton, clearButton]));

            const predictRow = el('div', { class: 'field-grid' }, []);
            const predictField = field('代入 x（可选）', 'x', { type: 'number', placeholder: '预测对应的 y' });
            predictRow.appendChild(predictField.node);
            panelNode.appendChild(predictRow);

            const message = new Message();
            const results = new ResultPanel();
            const calcButton = el('button', { class: 'primary', text: '计算' });

            calcButton.addEventListener('click', async () => {
                calcButton.disabled = true;
                message.clear();
                results.clear();
                try {
                    const points = [];
                    for (const input of body.querySelectorAll('input')) {
                        const value = input.value.trim();
                        if (value === '') continue;
                        points.push(value);
                    }
                    if (points.length < 2) throw new Error('请至少输入一对完整的 x 与 y。');

                    const params = { points: points.join(',') };
                    if (predictField.input.value.trim() !== '') params.x = predictField.input.value.trim();

                    const data = await api('/api/regress', params);
                    if (data.ok === false) {
                        message.error(data.error);
                        return;
                    }
                    const rows = [
                        ['样本数 n', fmt(data.count)],
                        ['x 的平均值', fmt(data.meanX)],
                        ['y 的平均值', fmt(data.meanY)],
                        ['回归斜率 k', fmt(data.slope), true],
                        ['回归截距 b', fmt(data.intercept), true],
                        ['相关系数 r', fmt(data.r)],
                        ['决定系数 R²', fmt(data.rSquared)],
                        ['x 标准差', fmt(data.stdDevX)],
                        ['y 标准差', fmt(data.stdDevY)],
                        ['k 的标准误', fmt(data.slopeStdError)],
                        ['回归方程', `y = ${fmt(data.slope)}x + ${fmt(data.intercept)}`],
                    ];
                    if (data.predictX !== undefined) {
                        rows.push(['预测值', `x = ${fmt(data.predictX)} 时，y = ${fmt(data.predictY)}`]);
                    }
                    results.setMany(rows);
                } catch (error) {
                    message.error(error.message || String(error));
                } finally {
                    calcButton.disabled = false;
                }
            });

            panelNode.appendChild(el('div', { class: 'actions' }, [calcButton]));
            root.append(panelNode, message.root, results.root);

            // 暴露 runner，支持用查询参数预填并自动计算。
            // 参数约定与通用 panel() 一致：fillFromParams(inputs, params)。
            root.__runner = {
                inputs: {},
                fillFromParams(inputs, params) {
                    if (!params.has('points')) return false;
                    // 数据点用分号分隔的 "x,y"，例如 "1,2;2,4;3,6"。
                    const pairs = params.get('points')
                        .split(';')
                        .map((pair) => pair.trim())
                        .filter(Boolean);
                    if (pairs.length > rowCount) {
                        rowCount = pairs.length;
                        renderRows();
                    }
                    const rowInputs = body.querySelectorAll('input');
                    pairs.forEach((pair, index) => {
                        const [x, y] = pair.split(',');
                        if (rowInputs[index * 2]) rowInputs[index * 2].value = (x || '').trim();
                        if (rowInputs[index * 2 + 1]) rowInputs[index * 2 + 1].value = (y || '').trim();
                    });
                    if (params.has('x')) predictField.input.value = params.get('x');
                    return true;
                },
                run: () => calcButton.click(),
            };
            return root;
        },
    },
    {
        id: 'equation',
        icon: '√',
        title: '方程求解',
        build() {
            const root = page('方程求解', '求解一次方程、二次方程（标准式与顶点式）以及任意次数的多项式。判别式为负时给出共轭复根。');

            const linear = panel('一次方程 ax + b = 0', [
                { label: 'a', key: 'la', type: 'number' },
                { label: 'b', key: 'lb', type: 'number' },
            ], async (values, results) => {
                const data = await api('/api/linear', { a: values.la, b: values.lb });
                if (!data.valid) { results.set('结果', data.reason); return; }
                results.setMany([['解', data.rootsText, true]]);
            });
            root.appendChild(linear.root);

            const quadratic = panel('二次方程 ax² + bx + c = 0', [
                { label: 'a', key: 'qa', type: 'number' },
                { label: 'b', key: 'qb', type: 'number' },
                { label: 'c', key: 'qc', type: 'number' },
            ], async (values, results) => {
                const data = await api('/api/quadratic', { a: values.qa, b: values.qb, c: values.qc });
                if (!data.valid) { results.set('结果', data.reason); return; }
                const rows = [
                    ['判别式 Δ', fmt(data.discriminant)],
                    ['解', data.rootsText, true],
                ];
                if (!data.hasRealRoots) rows.push(['说明', '该方程无实根，上面给出的是共轭复根']);
                results.setMany(rows);
            });
            root.appendChild(quadratic.root);

            const vertex = panel('二次方程 a(x − h)² + k = 0', [
                { label: 'a', key: 'va', type: 'number' },
                { label: 'h', key: 'vh', type: 'number' },
                { label: 'k', key: 'vk', type: 'number' },
            ], async (values, results) => {
                const data = await api('/api/vertex', { a: values.va, h: values.vh, k: values.vk });
                results.setMany([
                    ['展开式', `${fmt(data.expandedB)}x + ${fmt(data.expandedC)}`],
                    ['判别式 Δ', fmt(data.discriminant)],
                    ['解', data.rootsText, true],
                ]);
            });
            root.appendChild(vertex.root);

            const poly = panel('多项式求根（系数按降幂排列）', [
                { label: '系数（空格分隔）', key: 'coefficients', placeholder: '例如 1 -6 11 -6  表示 x³ − 6x² + 11x − 6', wide: true },
            ], async (values, results) => {
                const data = await api('/api/polynomial', { coefficients: values.coefficients });
                if (!data.valid) { results.set('结果', data.reason); return; }
                const rows = data.roots.map((root, index) =>
                    [`根 x${index + 1}`, fmtComplex(root), true]);
                results.setMany([['根的个数', fmt(data.count)], ...rows]);
            });
            root.appendChild(poly.root);

            return root;
        },
    },
    {
        id: 'complex',
        icon: '∿',
        title: '复数运算',
        build() {
            const root = page('复数运算', '计算复数的模与辐角、整数次幂，以及 n 次根。');
            const p = panel('输入', [
                { label: '实部', key: 'real', type: 'number', value: '1' },
                { label: '虚部', key: 'imag', type: 'number', value: '1' },
                { label: '幂（可选）', key: 'power', type: 'number', placeholder: '例如 3' },
                { label: 'n 次根（可选）', key: 'roots', type: 'number', placeholder: '例如 3' },
            ], async (values, results) => {
                const params = { real: values.real, imag: values.imag };
                if (values.power) params.power = values.power;
                if (values.roots) params.roots = values.roots;
                const data = await api('/api/complex', params);
                const rows = [
                    ['原复数', fmtComplex({ real: data.real, imag: data.imag })],
                    ['模', fmt(data.modulus), true],
                    ['辐角（弧度）', fmt(data.argument)],
                ];
                if (data.powerReal !== undefined) {
                    rows.push([`${data.power} 次幂`, fmtComplex({ real: data.powerReal, imag: data.powerImag }), true]);
                }
                if (data.roots) {
                    data.roots.forEach((root, index) => {
                        rows.push([`第 ${index + 1} 个根`, fmtComplex(root)]);
                    });
                }
                results.setMany(rows);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'matrix',
        icon: '▦',
        title: '矩阵运算',
        build() {
            const root = page('矩阵运算', '输入矩阵元素（按行展开），得到行列式、秩、转置与逆矩阵。');
            const p = panel('输入', [
                { label: '行数', key: 'rows', type: 'number', value: '2' },
                { label: '列数', key: 'cols', type: 'number', value: '2' },
                { label: '元素（逗号分隔，按行展开）', key: 'values', placeholder: '例如 1,2,3,4', wide: true },
            ], async (values, results) => {
                const data = await api('/api/matrix', {
                    rows: values.rows, cols: values.cols, values: values.values,
                });
                const rows = [
                    ['矩阵维度', `${data.rows} × ${data.cols}`],
                    ['行列式', fmt(data.determinant), true],
                    ['秩', fmt(data.rank)],
                    ['是否对称', data.isSymmetric ? '是' : '否'],
                ];
                if (data.invertible) {
                    const cols = data.cols;
                    const lines = [];
                    for (let r = 0; r < data.rows; r++) {
                        lines.push('[' + data.inverse.slice(r * cols, (r + 1) * cols).map((v) => fmt(v)).join(', ') + ']');
                    }
                    rows.push(['逆矩阵', lines.join('\n')]);
                } else {
                    rows.push(['逆矩阵', '不存在（矩阵奇异）']);
                }
                rows.push(['转置', data.transpose.join(', ')]);
                results.setMany(rows);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'system',
        icon: '⇉',
        title: '线性方程组',
        build() {
            const root = page('线性方程组', '用高斯-约当消元求解 n 元一次方程组，并判断解的唯一性。');
            const p = panel('输入', [
                { label: '系数矩阵（按行展开）', key: 'a', placeholder: '例如 1,1,1,-1', wide: true },
                { label: '右端项 b', key: 'b', placeholder: '例如 3,1', wide: true },
            ], async (values, results) => {
                const data = await api('/api/solve-system', { a: values.a, b: values.b });
                if (!data.valid) { results.set('结果', data.reason); return; }
                const rows = [
                    ['解', data.solution.map((v) => fmt(v)).join(', '), true],
                ];
                if (data.hasInfiniteSolutions) {
                    rows.push(['解的个数', '无穷多解']);
                    rows.push(['自由变量', data.freeVariables.map((i) => `x${i + 1}`).join(', ')]);
                } else {
                    rows.push(['解的个数', '唯一解']);
                }
                results.setMany(rows);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'triangle',
        icon: '△',
        title: '三角形',
        build() {
            const root = page('三角形计算', '输入空间中三角形的三个顶点坐标，得到面积、周长、三边长与最大高。三点共线时提示退化。');
            const p = panel('顶点坐标', [
                { label: 'P1 x, y, z', key: 'p1', placeholder: '0, 0, 0', wide: true },
                { label: 'P2 x, y, z', key: 'p2', placeholder: '3, 0, 0', wide: true },
                { label: 'P3 x, y, z', key: 'p3', placeholder: '0, 4, 0', wide: true },
            ], async (values, results) => {
                const points = [values.p1, values.p2, values.p3].join(',');
                const data = await api('/api/triangle', { points });
                if (data.degenerate) {
                    results.setMany([
                        ['退化三角形', '三点共线或重合，面积为 0'],
                        ['周长', fmt(data.perimeter)],
                    ]);
                    return;
                }
                results.setMany([
                    ['面积', fmt(data.area), true],
                    ['周长', fmt(data.perimeter), true],
                    ['边长 |P2P3|', fmt(data.sideA)],
                    ['边长 |P1P3|', fmt(data.sideB)],
                    ['边长 |P1P2|', fmt(data.sideC)],
                    ['最大高', fmt(data.altitude)],
                ]);
            });
            root.appendChild(p.root);
            root.__runner = p;
            return root;
        },
    },
    {
        id: 'tetrahedron',
        icon: '△',
        title: '空间几何',
        build() {
            const root = page('空间几何', '计算四面体的体积、表面积、重心与内外接球，以及球、圆柱、圆锥、棱柱、棱锥的表面积与体积。');

            const tetra = panel('四面体（四个顶点坐标）', [
                { label: 'P1 x, y, z', key: 't1', placeholder: '0, 0, 0', wide: true },
                { label: 'P2 x, y, z', key: 't2', placeholder: '1, 0, 0', wide: true },
                { label: 'P3 x, y, z', key: 't3', placeholder: '0, 1, 0', wide: true },
                { label: 'P4 x, y, z', key: 't4', placeholder: '0, 0, 1', wide: true },
            ], async (values, results) => {
                const points = [values.t1, values.t2, values.t3, values.t4].join(',');
                const data = await api('/api/tetrahedron', { points });
                if (data.degenerate) {
                    results.setMany([['退化四面体', data.reason]]);
                    return;
                }
                results.setMany([
                    ['体积', fmt(data.volume), true],
                    ['表面积', fmt(data.surfaceArea), true],
                    ['重心', data.centroid.map((v) => fmt(v)).join(', ')],
                    ['外接球球心', data.circumcenter.map((v) => fmt(v)).join(', ')],
                    ['外接球半径', fmt(data.circumradius)],
                    ['内切球半径', fmt(data.inradius)],
                ]);
            });
            root.appendChild(tetra.root);

            const solid = panel('常见几何体', [
                {
                    label: '类型', key: 'kind', type: 'select',
                    options: [
                        ['sphere', '球'], ['cylinder', '圆柱'], ['cone', '圆锥'],
                        ['prism', '正棱柱'], ['pyramid', '正棱锥'],
                    ],
                    value: 'sphere',
                },
                { label: '半径 r', key: 'radius', type: 'number', value: '2' },
                { label: '高 h', key: 'height', type: 'number', value: '3' },
                { label: '底面边数 n', key: 'sides', type: 'number', value: '6' },
            ], async (values, results) => {
                const params = { kind: values.kind, radius: values.radius };
                if (values.kind !== 'sphere') params.height = values.height;
                if (values.kind === 'prism' || values.kind === 'pyramid') params.sides = values.sides;
                const data = await api('/api/solid', params);
                results.setMany([
                    ['类型', data.kind],
                    ['表面积', fmt(data.surfaceArea), true],
                    ['体积', fmt(data.volume), true],
                ]);
            });

            root.appendChild(solid.root);
            return root;
        },
    },
    {
        id: 'sequences',
        icon: '⁂',
        title: '数列与组合',
        build() {
            const root = page('数列与组合', '查询斐波那契、卢卡斯、卡特兰数、阶乘与整数分��数，以及组合数与排列数。');

            const seq = panel('数列', [
                { label: '项号 n', key: 'n', type: 'number', value: '10' },
            ], async (values, results) => {
                const data = await api('/api/sequences', { n: values.n });
                results.setMany([
                    ['斐波那契 F(n)', data.fibonacci === null ? '超出 64 位整数范围' : fmt(data.fibonacci), true],
                    ['卢卡斯 L(n)', data.lucas === null ? '超出 64 位整数范围' : fmt(data.lucas)],
                    ['卡特兰数 C(n)', data.catalan === null ? '超出范围' : fmt(data.catalan)],
                    ['阶乘 n!', data.factorial === null ? '超出范围' : fmt(data.factorial)],
                    ['整数分拆数 p(n)', data.partition === null ? '超出范围' : fmt(data.partition)],
                ]);
            });
            root.appendChild(seq.root);

            const comb = panel('组合数', [
                { label: 'n', key: 'n', type: 'number', value: '10' },
                { label: 'k', key: 'k', type: 'number', value: '3' },
            ], async (values, results) => {
                const data = await api('/api/combinatorics', { n: values.n, k: values.k });
                results.setMany([
                    ['组合数 C(n, k)', data.binomial === null ? '超出范围' : fmt(data.binomial), true],
                    ['排列数 A(n, k)', data.permutation === null ? '超出范围' : fmt(data.permutation)],
                ]);
            });
            root.appendChild(comb.root);
            return root;
        },
    },
];

// ---------------------------------------------------------------- 应用启动

function buildNav() {
    const nav = document.getElementById('nav');
    const pages = document.getElementById('pages');
    const buttons = [];
    const built = [];

    PAGES.forEach((definition, index) => {
        const button = el('button', { class: 'nav-item', type: 'button' }, [
            el('span', { class: 'nav-icon', text: definition.icon }),
            el('span', { text: definition.title }),
        ]);
        button.addEventListener('click', () => activate(index));
        nav.appendChild(button);
        buttons.push(button);

        const pageNode = definition.build();
        pageNode.style.display = 'none';
        pages.appendChild(pageNode);
        built.push(pageNode);
    });

    function activate(index) {
        buttons.forEach((button, i) => button.classList.toggle('active', i === index));
        for (const child of pages.children) child.style.display = 'none';
        pages.children[index].style.display = '';
        window.location.hash = PAGES[index].id;
    }

    // 支持用查询参数预填并自动计算，便于分享链接与自动化测试。
    const params = new URLSearchParams(window.location.search);
    if (params.has('auto')) {
        for (const pageNode of built) {
            const runner = pageNode.__runner;
            if (!runner) continue;
            if (runner.fillFromParams(runner.inputs || {}, params)) {
                runner.run();
            }
        }
    }

    return activate;
}

async function checkHealth() {
    const status = document.getElementById('status-text');
    const dot = status.previousElementSibling;
    try {
        const data = await api('/api/health');
        status.textContent = `服务正常 · v${data.version}`;
        dot.className = 'dot';
    } catch (error) {
        status.textContent = '服务不可用';
        dot.className = 'dot error';
    }
}

document.addEventListener('DOMContentLoaded', () => {
    const activate = buildNav();

    // 若 URL 带 hash，直接跳到对应页面。
    const hash = window.location.hash.replace('#', '');
    const index = PAGES.findIndex((p) => p.id === hash);
    activate(index >= 0 ? index : 0);

    checkHealth();
});