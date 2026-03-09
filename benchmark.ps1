param(
    [int]$K = 4,
    [switch]$Only7z,
    [switch]$Skip7z
)

$ErrorActionPreference = "Stop"

$SEVENZIP    = "C:\\Program Files\\7-Zip\\7z.exe"
$OUT_DIR     = "saidas"
$SILESIA_DIR = "silesia"

$RESULT_7Z   = Join-Path $OUT_DIR "benchmark_7z.csv"
$RESULT_PPMC = Join-Path $OUT_DIR ("benchmark_ppmc_k{0}.csv" -f $K)
$LOG         = Join-Path $OUT_DIR ("ppm_k{0}.log" -f $K)

if (!(Test-Path $OUT_DIR)) {
    New-Item -ItemType Directory -Path $OUT_DIR | Out-Null
}

if (!(Test-Path $SILESIA_DIR)) {
    Write-Host "ERRO: pasta '$SILESIA_DIR' nao encontrada."
    exit 1
}

$culture = [System.Globalization.CultureInfo]::InvariantCulture

function Write-CsvHeader {
    param([string]$Path)
    "Tool,K,Arquivo,Original_bytes,Compressed_bytes,BPS,T_comp_s,T_decomp_s,Status" |
        Out-File -FilePath $Path -Encoding UTF8
}

function Run-7ZipBenchmark {
    param([string]$ResultPath)

    Write-Host "`n=== Benchmark 7z PPMd ==="
    Write-CsvHeader $ResultPath

    $totalOrig = 0
    $totalComp = 0
    $totalTComp = 0.0
    $totalTDecomp = 0.0

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
            Add-Content $ResultPath "7z-PPMd,-,$nome,$orig,-,-,-,-,ERRO_COMP"
            return
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

        $tComp   = [math]::Round($tempoComp.TotalSeconds, 6)
        $tDecomp = [math]::Round($tempoDecomp.TotalSeconds, 6)

        $totalOrig    += $orig
        $totalComp    += $comp
        $totalTComp   += $tComp
        $totalTDecomp += $tDecomp

        $bpsStr     = $bps.ToString($culture)
        $tCompStr   = $tComp.ToString($culture)
        $tDecompStr = $tDecomp.ToString($culture)

        "7z-PPMd,-,$nome,$orig,$comp,$bpsStr,$tCompStr,$tDecompStr,OK" |
            Add-Content -Path $ResultPath
    }

    if ($totalOrig -gt 0) {
        $totalBps = [math]::Round(($totalComp * 8.0) / $totalOrig, 6)

        $bpsStr     = $totalBps.ToString($culture)
        $tCompStr   = ([math]::Round($totalTComp, 6)).ToString($culture)
        $tDecompStr = ([math]::Round($totalTDecomp, 6)).ToString($culture)

        "7z-PPMd,-,TOTAL,$totalOrig,$totalComp,$bpsStr,$tCompStr,$tDecompStr,OK" |
            Add-Content -Path $ResultPath
    }

    Write-Host "CSV 7z: $ResultPath"
}

function Run-PPMCBenchmark {
    param(
        [int]$KValue,
        [string]$ResultPath,
        [string]$LogPath
    )

    Write-Host "=== Compilando com g++ -O3 ==="
    g++ -O3 -std=c++17 main.cpp src/*.cpp -o main.exe
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERRO na compilacao!"
        exit 1
    }

    Write-CsvHeader $ResultPath

    Write-Host "`n=== Benchmark PPM-C com K = $KValue ==="
    & ".\\main.exe" $KValue $ResultPath 2>&1 | Tee-Object -FilePath $LogPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERRO ao rodar main.exe com K=$KValue"
        exit 1
    }

    Write-Host "`n=== Gerando graficos para K = $KValue ==="
    python .\\plotar_benchmark.py $KValue $ResultPath
    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERRO ao gerar os graficos com K=$KValue"
        exit 1
    }

    Write-Host "CSV PPM-C: $ResultPath"
    Write-Host "LOG: $LogPath"
}

if ($Only7z -and $Skip7z) {
    Write-Host "ERRO: use apenas um entre -Only7z e -Skip7z."
    exit 1
}

if ($Only7z) {
    Run-7ZipBenchmark -ResultPath $RESULT_7Z
    exit 0
}

if (!$Skip7z) {
    Run-7ZipBenchmark -ResultPath $RESULT_7Z
}

Run-PPMCBenchmark -KValue $K -ResultPath $RESULT_PPMC -LogPath $LOG

Write-Host "`n=== Concluido com sucesso ==="
