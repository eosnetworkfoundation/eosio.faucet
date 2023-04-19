import crypto from "crypto";

let address = "0xaa2F34E41B397aD905e2f48059338522D05CA534";
address = address.slice(2);
const sha256 = crypto.createHash('sha256').update(address).digest('hex');
console.log({address, sha256});