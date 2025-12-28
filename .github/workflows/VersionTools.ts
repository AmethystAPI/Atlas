interface Config {
    meta: {
        version: string,
    }
}

const filePath = "./mod.json";
const modConfig: Config = JSON.parse(await Deno.readTextFile(filePath));

function bumpVersion(version: string) {
    const parts = version.split(".").map(Number);
    parts[2]++; 
    return parts.join(".");
}

modConfig.meta.version = bumpVersion(modConfig.meta.version);
Deno.writeTextFile(filePath, JSON.stringify(modConfig, null, 4));

// Output the new version for GitHub Actions
console.log(modConfig.meta.version);