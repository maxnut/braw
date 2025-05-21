Prism.languages.braw = {
    'comment': {
        pattern: /\/\/.*/g,
        greedy: true
    },
    'string': {
        pattern: /(["'])(?:\\.|(?!\1)[^\\\r\n])*\1/,
        greedy: true
    },
    'keyword': /\b(?:fn|let|return|if|else|while|do|for|define|struct|void|int|uint|long|ulong|char|uchar|float|double|bool|true|false|import)\b/,
    'boolean': /\b(?:true|false)\b/,
    'number': /\b(?:0x[\da-fA-F]+|0b[01]+|\d+(\.\d+)?(f|l|ul)?)\b/,
    'operator': /[-+*/%=!<>]=?|&&|\|\|/,
    'punctuation': /[{}[\];(),.:]/,
    'function': /\b\w+(?=\s*\()/,
    'macro': {
        pattern: /\$\w+|\#\w+|\$make_\w+|\$concat|\$foreach/,
        alias: 'important'
    }
};
