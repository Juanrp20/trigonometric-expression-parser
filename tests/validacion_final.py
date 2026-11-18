import subprocess, math, re
cases = [
('E01','sin(3.14/2)+cos(0)*2^2',[], math.sin(3.14/2)+4),
('E02','sin(x)+cos(y)',[0,0],1),
('E03','sin(cos(x))+tan(y)/cos(x)',[0,0],math.sin(1)),
('E04','cos(x)^2+sin(x)^2',[1],1),
('E05','sin(x)',[1],math.sin(1)),
('E06','cos(x+y)',[1,2],math.cos(3)),
('E07','tan(x^2+sin(x))',[1],math.tan(1+math.sin(1))),
('A01','2+3*4',[],14),('A02','8-3-1',[],4),('A03','8/4/2',[],1),
('A04','2^3^2',[],512),('A05','-2^2',[],-4),('A06','2^-2',[],.25),('A07','1--2',[],3),('A08','x+x',[3],6),
]
for ident,expr,vals,expected in cases:
    inp=expr+'\n'+''.join(str(v)+'\n' for v in vals)
    p=subprocess.run(['./calculadora_expr'], input=inp, text=True, capture_output=True, timeout=5)
    m=re.search(r'Resultado: ([^\n]+)',p.stdout)
    assert p.returncode==0 and m,(ident,p.stdout,p.stderr,p.returncode)
    result=float(m.group(1)); assert math.isclose(result,expected,rel_tol=1e-12,abs_tol=1e-12),(ident,result,expected)
    print(ident,expr,'vars=',vals,'esperado=',format(expected,'.17g'),'obtenido=',m.group(1),'PASS')
for x in [-7,-1,0,.5,3.14,100]:
    p=subprocess.run(['./calculadora_expr'],input=f'cos(x)^2+sin(x)^2\n{x}\n',text=True,capture_output=True,timeout=5)
    value=float(re.search(r'Resultado: ([^\n]+)',p.stdout).group(1))
    assert p.returncode==0 and math.isclose(value,1,rel_tol=1e-12,abs_tol=1e-12)
    print('IDENTIDAD',x,format(value,'.17g'),'PASS')
for expr in ['sin()','sin(x+)','cos(x','2+','2**3','sin(1,2)','foo(1)','x(2)','2 3','sin(x) texto']:
    p=subprocess.run(['./calculadora_expr'],input=expr+'\n',text=True,capture_output=True,timeout=5)
    assert p.returncode==2 and 'Valor de' not in p.stdout
    print('INVALIDA',expr,'codigo=',p.returncode,'error=',p.stderr.strip(),'PASS')
for expr in ['1/0','0^-1','(-1)^0.5','10^1000']:
    p=subprocess.run(['./calculadora_expr'],input=expr+'\n',text=True,capture_output=True,timeout=5)
    assert p.returncode==4
    print('MATEMATICA',expr,'codigo=',p.returncode,'error=',p.stderr.strip(),'PASS')
