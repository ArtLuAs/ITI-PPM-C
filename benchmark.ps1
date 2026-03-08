param(
    [int]$K = 4
)

$ErrorActionPreference = "Stop"

$SEVENZIP    = "C:\\Program Files\\7-Zip\\7z.exe"
$RESULT      = "saidas\\benchmark_resultados.csv"
$OUT_DIR     = "saidas"
$SILESIA_DIR = "silesia"
$LOG         = Join-Path $OUT_DIR "ppm_k${K}.log"

if (!(Test-Path $OUT_DIR)) {
    New-Item -ItemType Directory -Path $OUT_DIR | Out-Null
}

if (!(Test-Path $SILESIA_DIR)) {
    Write-Host "ERRO: pasta '$SILESIA_DIR' nao encontrada."
    exit 1
}

Write-Host "=== Compilando com g++ -O3 ==="
g++ -O3 -std=c++17 main.cpp src/*.cpp -o main.exe
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO na compilacao!"
    exit 1
}

"Tool,K,Arquivo,Original_bytes,Compressed_bytes,BPS,T_comp_s,T_decomp_s,Status" |
    Out-File -FilePath $RESULT -Encoding UTF8

Write-Host "`n=== Benchmark 7z PPMd ==="

# Variáveis para acumular os totais do 7-Zip
$totalOrig = 0
$totalComp = 0
$totalTComp = 0.0
$totalTDecomp = 0.0
$culture = [System.Globalization.CultureInfo]::InvariantCulture

Get-ChildItem $SILESIA_DIR -File | ForEach-Object {
    $arquivo = $_.FullName
    $nome    = $_.Name
    $orig    = $_.Length
    $saida7z = Join-Path $OUT_DIR ($nome + ".7z")

    if (Test-Path $saida7z) {
        Remove-Item $saida7z -Force
    }

    $tempoComp = Measure-Command {
        & $SEVENZIP a -t7z -m0=PPMd -mx=9 $saida7z $arquivo | Out-Null
    }

    if (!(Test-Path $saida7z)) {
        Add-Content $RESULT "7z-PPMd,-,$nome,$orig,-,-,-,-,ERRO_COMP"
        return # Funciona como 'continue' no ForEach-Object
    }

    $comp = (Get-Item $saida7z).Length
    $bps  = if ($orig -gt 0) { [math]::Round(($comp * 8.0) / $orig, 6) } else { 0 }

    $tempDir = Join-Path $OUT_DIR ("tmp_" + $nome)
    if (Test-Path $tempDir) {
        Remove-Item $tempDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $tempDir | Out-Null

    $tempoDecomp = Measure-Command {
        & $SEVENZIP x $saida7z "-o$tempDir" -y | Out-Null
    }

    Remove-Item $tempDir -Recurse -Force
    Remove-Item $saida7z -Force

    $tComp   = $tempoComp.TotalSeconds
    $tDecomp = $tempoDecomp.TotalSeconds

    # Acumulando para a linha de TOTAL
    $totalOrig    += $orig
    $totalComp    += $comp
    $totalTComp   += $tComp
    $totalTDecomp += $tDecomp

    $bpsStr     = $bps.ToString($culture)
    $tCompStr   = [math]::Round($tComp, 6).ToString($culture)
    $tDecompStr = [math]::Round($tDecomp, 6).ToString($culture)

    "7z-PPMd,-,$nome,$orig,$comp,$bpsStr,$tCompStr,$tDecompStr,OK" |
        Add-Content -Path $RESULT
}

# Gravando a linha com o TOTAL do 7-Zip
if ($totalOrig -gt 0) {
    $totalBps = [math]::Round(($totalComp * 8.0) / $totalOrig, 6)
    
    $bpsStr     = $totalBps.ToString($culture)
    $tCompStr   = [math]::Round($totalTComp, 6).ToString($culture)
    $tDecompStr = [math]::Round($totalTDecomp, 6).ToString($culture)

    "7z-PPMd,-,TOTAL,$totalOrig,$totalComp,$bpsStr,$tCompStr,$tDecompStr,OK" |
        Add-Content -Path $RESULT
}


Write-Host "`n=== Benchmark PPM-C com K = $K ==="
& ".\\main.exe" $K $RESULT 2>&1 | Tee-Object -FilePath $LOG
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO ao rodar main.exe com K=$K"
    exit 1
}

Write-Host "`n=== Gerando graficos para K = $K ==="
python .\\plotar_benchmark.py $K
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERRO ao gerar os graficos com K=$K"
    exit 1
}

Write-Host "`n=== Concluido com sucesso para K = $K ==="
Write-Host "CSV: $RESULT"
Write-Host "LOG: $LOG"
