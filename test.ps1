Push-Location
# Store the parent directory path
$parentDir = Split-Path -Parent 'C:\Users\Caleb Lamoreaux\Downloads\P3\tests'

# Change the directory to the /tests/ folder
cd 'C:\Users\Caleb Lamoreaux\Downloads\P3\tests'

# Get all .txt files in the /tests/ folder, excluding .expected.txt files
$txtFiles = Get-ChildItem -Filter *.txt | Where-Object { $_.Name -notlike "*.expected.txt" }

foreach ($file in $txtFiles) {
    # Extract the filename without the .txt extension
    $baseName = $file.BaseName

    # Display the test being run
    Write-Host ""
    Write-Host "RUNNING ${baseName}"

    # Run a.exe from the parent directory with the current file as input and capture the output
    $inputFileContent = Get-Content $file.FullName -Raw
    $programOutput = $inputFileContent | & "$parentDir\a.exe" | Out-String
    Write-Host "-----------------------------------"
    Write-Host "Program Output"
    Write-Host "-----------------------------------"
    Write-Host $programOutput

    # Get the expected output file name
    $expectedOutputFile = "$baseName.txt.expected"

    # Check if the expected output file exists
    if (Test-Path $expectedOutputFile) {
        $expectedOutput = Get-Content $expectedOutputFile | Out-String
        Write-Host "-----------------------------------"
        Write-Host "Expected Output"
        Write-Host "-----------------------------------"
        Write-Host $expectedOutput
        # Perform the diff
        $diff = Compare-Object -ReferenceObject $programOutput -DifferenceObject $expectedOutput

        if ($diff) {
            Write-Host "FAILED: Differences found for ${baseName}"
        } else {
            Write-Host "PASSED: No differences found for ${baseName}"
        }
        Write-Host ""
    } else {
        Write-Host "Expected output file not found for ${baseName}"
    }

    Write-Host "COMPLETED ${baseName}"
    Write-Host "" # Add a blank line for readability
}
Pop-Location