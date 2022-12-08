#!/usr/bin/env node
// #!/usr/bin/env deno run -A  consume.js
// the JavaScript ecosystem is so broken that none of this works

//  https://wasmer.io/posts/wasmer-takes-webassembly-libraries-manistream-with-wai

import {bindings} from "wai/tutorial-01";

async function main() {
	const calculator = await bindings.calculator();
	const four = calculator.add(2, 2);
	console.log("2 + 2 =", four);
}

main();
